precision mediump float;
uniform sampler2D texture1;
uniform vec4 colObject;
varying vec2 varTexCoord;
void main() {
  gl_FragColor = texture2D(texture1, varTexCoord)*colObject;
}