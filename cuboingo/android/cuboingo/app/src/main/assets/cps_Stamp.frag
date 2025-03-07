precision mediump float;
uniform sampler2D texture1;
varying vec2 varTexCoord;
varying vec4 varColor;
void main() {
  gl_FragColor = texture2D(texture1, varTexCoord)*varColor;
}