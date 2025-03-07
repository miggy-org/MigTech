#pragma once

///////////////////////////////////////////////////////////////////////////
// platform specific

#include "../core/MigDefines.h"
#include "../core/Object.h"
#include "DxShader.h"
#include "DxImage.h"

namespace MigTech
{
	struct ShaderSet
	{
		DxShader* vertexShader;
		DxShader* pixelShader;
	};

	struct TxtMapping
	{
		DxImage* pimg;
		ID3D11SamplerState* pstate;
	};

	// Windows DirectX version of a MigTech object
	class DxObject : public Object
	{
	protected:
		std::vector<ShaderSet> _shaderSets;

		Microsoft::WRL::ComPtr<ID3D11Buffer> _vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> _indexBuffer;

		unsigned int _vertexStride;
		unsigned int _vertexOffset;
		unsigned int _indexCount;
		unsigned int _indexOffset;
		unsigned int _indexOffsetCount;
		D3D_PRIMITIVE_TOPOLOGY _topology;

		// texture mappings
		std::vector<TxtMapping> _mappings;

		bool _inRenderSet;

	protected:
		void prepareRender(int shaderSet);

	public:
		DxObject();
		virtual ~DxObject();

		virtual int addShaderSet(const std::string& vs, const std::string& ps);
		virtual void setImage(int index, const std::string& name, TXT_FILTER minFilter, TXT_FILTER magFilter, TXT_WRAP wrap);
		virtual void loadVertexBuffer(const void* pdata, unsigned int count, VDTYPE vdType);
		virtual void loadIndexBuffer(const unsigned short* indices, unsigned int count, PRIMITIVE_TYPE type);

		virtual void setIndexOffset(unsigned int offset, unsigned int count);
		virtual int getIndexOffset() const;
		virtual int getIndexCount() const;

		virtual void render(int shaderSet = 0);

		virtual void startRenderSet(int shaderSet = 0);
		virtual void stopRenderSet();
	};
}