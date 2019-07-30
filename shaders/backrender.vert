#version 330

// Attribute変数
layout(location = 0) in vec3 in_position;


// Varying変数
out vec2 v_uv;


void main() {
    int idx = gl_VertexID;
    // gl_Positionは頂点シェーダの組み込み変数
    // 指定を忘れるとエラーになるので注意
    gl_Position = vec4(in_position, 1.0);
    // Varying変数への代入
    v_uv = vec2()gl_Position.xy * 0.5 +0.5);
}


