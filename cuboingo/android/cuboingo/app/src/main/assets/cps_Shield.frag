precision mediump float;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform vec4 miscVal;

varying vec2 varTexCoord;
varying vec4 varColor;
varying vec3 varNorm;
varying vec3 varPos;

vec3 getReflectionRay(vec3 eye, vec3 pos, vec3 norm)
{
  vec3 dir;
  float f2ndoti;

  dir = normalize(pos - eye);
  f2ndoti = 2.0 * dot(norm, dir);
  return (dir - f2ndoti*norm);
}

float convertToUV(float arg)
{
  return (arg / 1.4142) + 0.5;
}

vec2 getReflectionCoords(vec3 eye, vec3 pos, vec3 norm)
{
  vec3 ray;
  vec2 uv;
  float ax, ay, az;
	
  ray = getReflectionRay(eye, pos, norm);
  ax = abs(ray.x);
  ay = abs(ray.y);
  az = abs(ray.z);
  if (ax > ay && ax > az)
  {
    uv.x = ray.y;
    uv.y = ray.z;
  }
  else if (ay > ax && ay > az)
  {
    uv.x = ray.x;
    uv.y = ray.z;
  }
  else
  {
    uv.x = ray.x;
    uv.y = ray.y;
  }

  uv.x = convertToUV(uv.x);
  uv.y = convertToUV(uv.y);

  return uv;
}

void main() {
  vec4 retCol = varColor;
  if (miscVal.w > 0.0)
  {
    retCol *= 0.5;

    vec2 uv = getReflectionCoords(vec3(miscVal.x, miscVal.y, miscVal.z), varPos, varNorm);
    vec4 reflCol = texture2D(texture2, uv)*varColor;
    retCol += 0.5*reflCol;
  }
  gl_FragColor = retCol;
}