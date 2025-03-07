#pragma once

#include "MigBase.h"
#include "Object.h"
#include "AnimList.h"
#include "Matrix.h"

namespace MigTech
{
	class MovieClip;

	class IMovieClipCallback
	{
	public:
		// called when the movie clip animation is done playing
		virtual void movieComplete(MovieClip* mc) = 0;
	};

	struct UVSet
	{
		float u1, v1;
		float u2, v2;
	};

	class MovieClip : public MigBase, public IAnimTarget
	{
	public:
		MovieClip();

		// initialization (dimensions are world coordinates)
		bool init(const std::string& bmpID, int rowCount, int colCount, float mcW, float mcH, bool blendFrames);
		bool init(const std::string& bmpID, float mcW, float mcH);

		virtual void createGraphics();
		virtual void destroyGraphics();

		virtual void draw(const Matrix& worldMatrix, bool depth, float alpha = 1) const;
		virtual void draw(float alpha = 1) const;

		// IAnimTarget
		virtual bool doFrame(int id, float newVal, void* optData);
		virtual void animComplete(int id, void* optData);

		// getters/setters
		void setPos(const Vector3& pos) { _pos = pos; applyTransformations(); }
		void setRot(float rX, float rY, float rZ) { _rX = rX; _rY = rY; _rZ = rZ; applyTransformations(); }
		void setScale(float sX, float sY) { _sX = sX; _sY = sY; applyTransformations(); }
		void setColor(const Color& col) { _color = col; }
		void setAlpha(float alpha) { _color.a = alpha; }
		void setVisible(bool vis) { _visible = vis; }
		bool isVisible() const { return _visible; }
		bool isInit() const { return !_bmpResID.empty(); }

		// frame play/jump
		void jumpToFrame(float frame);
		void playToFrame(float frame, long duration, IMovieClipCallback* callback);

		// render sets
		void startRenderSet();
		void stopRenderSet();

	protected:
		void applyTransformations();
		void drawFrame(bool first, float param, float alpha) const;
		void drawFrames(bool depth, float alpha) const;

	protected:
		std::string _bmpResID;
		Object* _screenPoly1;
		Object* _screenPoly2;
		VertexPositionTexture* _txtVerts;

		int _rowCount;
		int _colCount;
		float _width;
		float _height;
		int _frameCount;
		bool _blendFrames;
		float _playHead;
		bool _visible;

		Matrix _mat;
		Vector3 _pos;
		float _rX, _rY, _rZ;
		float _sX, _sY;
		Color _color;

		AnimID _idAnim;
		UVSet _uv1, _uv2;
		IMovieClipCallback* _callback;
	};
}