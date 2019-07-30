#version 330

in vec3 f_positionCameraSpace;
in vec3 f_normalCameraSpace;
in vec3 f_lightPosCameraSpace;

out vec4 out_color;

uniform vec3 u_diffColor;
uniform vec3 u_specColor;
uniform vec3 u_ambiColor;
uniform float u_shininess;
uniform float u_roughness;

void main() {

	vec3 V = normalize(-f_positionCameraSpace);
	vec3 N = normalize(f_normalCameraSpace);
	vec3 L = normalize(f_lightPosCameraSpace - f_positionCameraSpace);
	

	// Oren-Nayarの反射モデル
	float sigma2 = u_roughness * u_roughness;
	float A = 1.0 - 0.5 * sigma2 / (sigma2 + 0.33);
	float B = 0.45 * sigma2 / (sigma2 + 0.09);

	float ndotl = max(0.0, dot(N, L));
	float ndotv = max(0.0, dot(N, V));
	float ldotv = dot(L, V);
	float sinThetaL = sqrt(max(0.0, 1.0 - ndotl * ndotl));
	float sinThetaV = sqrt(max(0.0, 1.0 - ndotv * ndotv));
	float s = ldotv - ndotl * ndotv;
	if (ndotl > ndotv){
		s = s / ndotl;
	} else {
		s = s / ndotv;
	}
	float result = A + B * s;
	
	vec3 diffuse = u_diffColor * ndotl * result;
	vec3 ambient = u_ambiColor;

    out_color = vec4(diffuse + ambient, 1.0);
}