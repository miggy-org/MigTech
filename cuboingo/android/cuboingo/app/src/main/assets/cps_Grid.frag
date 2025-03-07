precision mediump float;
uniform sampler2D texture1;
varying vec2 varTexCoord;
varying vec4 varColor;
void main() {
  if (varTexCoord.x >= 0.0)
    gl_FragColor = texture2D(texture1, varTexCoord)*varColor;
  else
    gl_FragColor = varColor;
}