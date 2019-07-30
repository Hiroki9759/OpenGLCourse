#version 330

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;

out vec3 f_fragColor;

uniform mat4 u_mvpMat;

void main() {
    // gl_Positionは頂点シェーダの組み込み変数
    // 指定を忘れるとエラーになるので注意
    gl_Position = u_mvpMat * vec4(in_position, 1.0);
    f_fragColor = in_normal * 0.5 + 0.5;
}