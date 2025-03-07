attribute vec4 vPosition;
attribute vec3 vNorm;
attribute vec2 vTex1;

uniform mat4 matMVP;
uniform mat4 matModel;
uniform vec4 colObject;
uniform vec4 dirPosLit1;
uniform ivec4 cfgVal;

varying vec2 varTexCoord;
varying vec4 varColor;
varying vec3 varNorm;
varying vec3 varPos;

void main() {
  // transform the vertex position into projected space
  gl_Position = matMVP * vPosition;

  // cfgVal.x is the rendering pass (0 = final)
  if (cfgVal.x == 0)
  {
    vec4 norm = matModel * vec4(vNorm, 0);
    float dotProd = -dot(vec3(dirPosLit1.x, dirPosLit1.y, dirPosLit1.z), vec3(norm.x, norm.y, norm.z));

    varColor = vec4(colObject.r*dotProd, colObject.g*dotProd, colObject.b*dotProd, colObject.a);
    varNorm = vec3(norm.x, norm.y, norm.z);
  }
  else
  {
    // pass through the color
    varColor = colObject;
    varNorm = vec3(0, 0, 0);
  }

  varPos = vec3(gl_Position.x, gl_Position.y, gl_Position.z);
  varTexCoord = vTex1;
}