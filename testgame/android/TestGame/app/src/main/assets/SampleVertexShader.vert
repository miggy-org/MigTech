attribute vec4 vPosition;
attribute vec4 vColor;
uniform mat4 matMVP;
uniform vec4 colObject;
varying vec4 varColor;
void main() {
  gl_Position = matMVP * vPosition;
  varColor = vColor * colObject;
}