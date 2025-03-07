attribute vec4 vPosition;
attribute vec2 vTex1;
uniform vec4 miscVal;
varying vec2 varTexCoord;
void main() {
  gl_Position = vPosition;
  varTexCoord = vTex1;
  varTexCoord.x += miscVal.x;
  varTexCoord.y += miscVal.y;
}