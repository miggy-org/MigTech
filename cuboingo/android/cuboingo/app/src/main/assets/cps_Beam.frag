precision mediump float;

uniform vec4 colObject;
uniform vec4 miscVal;

varying vec2 varTexCoord;

void main() {
  //gl_FragColor = colObject;
  vec4 retCol = colObject;
  retCol.a = 0.5 * varTexCoord.x * miscVal.x;
  if (miscVal.y > 0.0)
  {
    if (varTexCoord.y <= miscVal.x)
    {
      float range = 0.1;
      float base = miscVal.x - range;
      float val = (varTexCoord.y - base) / range;
      if (val > 0.0)
      {
        retCol.r = retCol.r + (1.0 - retCol.r)*val / 2.0;
        retCol.g = retCol.g + (1.0 - retCol.g)*val / 2.0;
        retCol.b = retCol.b + (1.0 - retCol.b)*val / 2.0;
        retCol.a *= (1.0 + val);
      }
    }
  }
  gl_FragColor = retCol;
}