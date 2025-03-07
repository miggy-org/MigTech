attribute vec4 vPosition;
attribute vec2 vTex1;
varying vec2 varTexCoord;
void main() {
  gl_Position = vPosition;
  varTexCoord = vTex1;
}