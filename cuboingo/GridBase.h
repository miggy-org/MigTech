#pragma once

#include "../core/MigInclude.h"
#include "../core/AnimList.h"
#include "../core/Object.h"
#include "../core/Matrix.h"
#include "CubeConst.h"

using namespace MigTech;

namespace Cuboingo
{
	class GridBase;

	class GridInfo
	{
	public:
		GridInfo();
		GridInfo(const GridInfo& rhs);
		~GridInfo();

		void operator=(const GridInfo& rhs);

		void init(AxisOrient newOrient, int newDimen, int newMapIndex);
		void initFromGrid(const GridBase& grid, bool incReservedInFillList);
		void setFillList(bool newValue);
		void copyFillList(const GridInfo& other);
		void invertFillList();
		void randomizeFillList(int maxFilled);
		void assignFillList(const int* compArray, int compValue);

		int fillListLen() const { return (fillList != nullptr ? dimen*dimen : 0); }

	public:
		int uniqueID;			// unique identifier (optional)
		AxisOrient orient;		// see AXISORIENT_*
		int dimen;				// dimension of this grid, currently only 1, 2 and 3 supported
		int mapIndex;			// texture map index
		Color emptyCol;			// empty color
		Color fillCol;			// filled color
		bool* fillList;		    // array of filled or reserved values, size is dimen^2
	};

	class Slot
	{
	public:
		Slot();

		void configSlot(float x, float y, float z);
		void configSlot(const Vector3& pt);
		void setColor(float r, float g, float b);
		void setColor(const Color& newColor);
		void copySlotInfo(const Slot& rhs);

	public:
		Vector3 ptCenter;
		Color color;
		bool filled;
		bool invis;
		bool animating;
		int reserved;
		float scale;
	};

	class GridBase
	{
		friend class GridInfo;

	public:
		enum FillState { FILLSTATE_EMPTY, FILLSTATE_PARTIAL, FILLSTATE_FILLED };

		// management of the static geometry
		static void createGraphics();
		static void destroyGraphics();

	public:
		GridBase();
		virtual ~GridBase();

		bool init(int dimension, int index, AxisOrient orientation, float dist);

		virtual void draw(const Matrix& mat) const;

		bool setEmptyColors(float r, float g, float b);
		bool setEmptyColors(const Color& col);
		bool setFilledColors(float r, float g, float b);
		bool setFilledColors(const Color& col);
		bool setAllColors(float er, float eg, float eb, float fr, float fg, float fb);
		bool setAllColors(const Color& eCol, const Color& fCol);
		bool isFilled(bool incReserved) const;
		bool isEmpty(bool incReserved) const;
		void reserveSlots(const GridInfo& info);
		void clearReserveSlots(int uniqueID);
		FillState getFillState(bool incReserved) const;

		AxisOrient getOrient() const { return _orient; }
		Color getEmptyColor() const { return _emptyCol; }
		Color getFillColor() const { return _fillCol; }
		int getMapIndex() const { return _mapIndex; }
		int getSlotCount() const { return _dimen*_dimen; }
		Slot& getSlot(int i) const { return _theSlots[i]; }

	protected:
		virtual void applyTransform(Matrix& outMatrix, const Matrix& worldMatrix, const Slot& theSlot) const;
		virtual void applyOffset(Matrix& mat) const;
		virtual const Color& getSlotDrawColor(const Slot& theSlot, int index) const;

		void updateSlotRadius(float newVal, bool onlyIfAnimating);
		void updateMapIndex(int newIndex);

	protected:
		AxisOrient _orient;		// see ORIENT_*
		int _dimen;				// dimension of this grid, currently only 1, 2 and 3 supported
		int _mapIndex;
		float _distFromOrigin;
		float _gridDepth;

		Color _emptyCol;
		Color _fillCol;
		float _slotRadius;
		float _slotRadiusScale;
		Slot* _theSlots;

		// shared amongst all instances
		static Object* _objFace;
		static Object* _objSides;
	};
}