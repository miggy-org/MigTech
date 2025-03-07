precision mediump float;

attribute vec4 vPosition;
attribute vec3 vNorm;
attribute vec2 vTex1;

uniform mat4 matMVP;
uniform mat4 matModel;
uniform vec4 colObject;
uniform vec4 dirPosLit1;

varying vec2 varTexCoord;
varying vec4 varColor;

void main() {
  gl_Position = matMVP * vPosition;
  varTexCoord = vTex1;

  vec4 norm = matModel * vec4(vNorm, 0);
  float dotProd = -dot(vec3(dirPosLit1.x, dirPosLit1.y, dirPosLit1.z), vec3(norm.x, norm.y, norm.z));

  varColor = vec4(colObject.r*dotProd, colObject.g*dotProd, colObject.b*dotProd, colObject.a);
}