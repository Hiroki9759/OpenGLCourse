
// #version 330

// // Varying変数
// in vec3 f_fragColor;

// // ディスプレイへの出力変数
// out vec4 out_color;

// // 選択を判定するためのID
// uniform int u_selectID;

// void main() {
//     if (u_selectID > 0) {
//         // 選択のIDが0より大きければIDで描画する
//         float c = u_selectID / 255.0;
//         out_color = vec4(c, c, c, 1.0);
//     } else {
//         // 描画色を代入
//         out_color = vec4(f_fragColor, 1.0);
//     }
// }
#version 330

// Varying変数
in vec2 f_texPos;

// ディスプレイへの出力変数
out vec4 out_color;

uniform sampler2D u_texture;
//選択を判別するためのID;
uniform int u_objectID;
void main() {
    // 描画色を代入
    if(u_objectID > 0){
        //選択のIDが0より大きければIDで描画する
        float c = u_objectID / 255.0;
        out_color = vec4(c, c, c, 1.0);
    } else {
        // 描画色を代入
        //out_color = vec4(f_fragColor, 1.0);
        out_color = vec4(texture(u_texture, f_texPos).rgb, 1.0);
    }
    
}