#pragma once

#include "MigDefines.h"

namespace MigTech
{
	class Object
	{
	protected:
		FACE_CULLING _cull;

	public:
		virtual int addShaderSet(const std::string& vs, const std::string& ps) = 0;
		virtual void setImage(int index, const std::string& name, TXT_FILTER minFilter, TXT_FILTER magFilter, TXT_WRAP wrap) = 0;
		virtual void setCulling(FACE_CULLING newCull)
		{
			_cull = newCull;
		};

		virtual void loadVertexBuffer(const void* pdata, unsigned int count, VDTYPE vdType) = 0;
		virtual void loadIndexBuffer(const unsigned short* indices, unsigned int count, PRIMITIVE_TYPE type) = 0;

		virtual void setIndexOffset(unsigned int offset, unsigned int count) = 0;
		virtual int getIndexOffset() const = 0;
		virtual int getIndexCount() const = 0;

		virtual void render(int shaderSet = 0) = 0;

		virtual void startRenderSet(int shaderSet = 0) = 0;
		virtual void stopRenderSet() = 0;
	};
}
