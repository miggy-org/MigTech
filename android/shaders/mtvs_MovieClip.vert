precision mediump float;

attribute vec4 vPosition;
attribute vec2 vTex1;

uniform mat4 matMVP;
uniform vec4 miscVal;

varying vec2 varTexCoord;

vec2 getUVSet(float frame, float rowCount, float colCount, vec2 uvInc)
{
  float row = floor(frame / colCount);
  float col = mod(frame, colCount);

  float u1 = (1.0 / colCount) * col;
  float v1 = (1.0 / rowCount) * row;
  float u2 = u1 + (1.0 / colCount);
  float v2 = v1 + (1.0 / rowCount);

  vec2 uvRet;
  uvRet.x = (uvInc.x == 0.0 ? u1 : u2);
  uvRet.y = (uvInc.y == 0.0 ? v1 : v2);
  return uvRet;
}

void main()
{
  gl_Position = matMVP * vPosition;

  // misc.x is the frame index
  // misc.y is the row count
  // misc.z is the column count
  if (miscVal.y > 1.0 || miscVal.z > 1.0)
    varTexCoord = getUVSet(miscVal.x, miscVal.y, miscVal.z, vTex1);
  else
    varTexCoord = vTex1;
}