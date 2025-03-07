#pragma once

#include "MigDefines.h"

namespace MigTech
{
	// built in vertex shaders
	static const std::string MIGTECH_VSHADER_PREFIX = "mtvs_";
	static const std::string MIGTECH_VSHADER_POS_NO_TRANSFORM = "mtvs_PosNoTransform";
	static const std::string MIGTECH_VSHADER_POS_TRANSFORM = "mtvs_PosTransform";
	static const std::string MIGTECH_VSHADER_POS_TEX_NO_TRANSFORM = "mtvs_PosTexNoTransform";
	static const std::string MIGTECH_VSHADER_POS_TEX_TRANSFORM = "mtvs_PosTexTransform";

	// built in pixel shaders
	static const std::string MIGTECH_PSHADER_PREFIX = "mtps_";
	static const std::string MIGTECH_PSHADER_COLOR = "mtps_Color";
	static const std::string MIGTECH_PSHADER_TEX = "mtps_Tex";
	static const std::string MIGTECH_PSHADER_TEX_ALPHA = "mtps_TexAlpha";

	class Shader
	{
	public:
		enum Type { SHADER_TYPE_NONE, SHADER_TYPE_VERTEX, SHADER_TYPE_PIXEL };

	protected:
		unsigned int _shaderHints;

	protected:
		Shader()
		{
			_shaderHints = SHADER_HINT_NONE;
		};
		Shader(unsigned int shaderHints)
		{
			_shaderHints = shaderHints;
		}

	public:
		virtual Type getType() = 0;
		virtual unsigned int getHints()
		{
			return _shaderHints;
		}
	};
}