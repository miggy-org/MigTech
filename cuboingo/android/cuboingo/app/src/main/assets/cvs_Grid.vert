precision mediump float;

attribute vec4 vPosition;
attribute vec3 vNorm;
attribute vec2 vTex1;

uniform mat4 matMVP;
uniform mat4 matModel;
uniform vec4 colObject;
uniform vec4 dirPosLit1;
uniform vec4 miscVal;

varying vec2 varTexCoord;
varying vec4 varColor;

void main() {
  gl_Position = matMVP * vPosition;
  varColor = colObject;
  //varTexCoord = vTex1;

  // misc.x is the map index (-1 = no map, 0-7 otherwise)
  if (miscVal.x != -1.0)
  {
    varTexCoord.x = 0.125*(miscVal.x + vTex1.x);
    varTexCoord.y = vTex1.y;
  }
  else
    varTexCoord.x = varTexCoord.y = -1.0;
}