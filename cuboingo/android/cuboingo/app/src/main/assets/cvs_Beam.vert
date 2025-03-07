attribute vec4 vPosition;
attribute vec2 vTex1;

uniform mat4 matMVP;
uniform vec4 colObject;

varying vec2 varTexCoord;

void main() {
  gl_Position = matMVP * vPosition;
  varTexCoord = vTex1;
}