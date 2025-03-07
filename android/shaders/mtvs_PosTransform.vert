attribute vec4 vPosition;
uniform mat4 matMVP;
uniform vec4 colObject;
void main() {
  gl_Position = matMVP * vPosition;
}