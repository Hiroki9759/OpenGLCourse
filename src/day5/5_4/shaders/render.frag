#version 330

// Varying変数
in vec3  f_fragColor;
in float f_dropId;
in vec2  f_texPos;
// ディスプレイへの出力変数
out vec4 out_color;

// 選択を判定するためのID
uniform int u_selectMode;
uniform int u_selectDrop;
uniform sampler2D u_texture;
//uniform int u_tyfediff[7] = {(0.0f,0.0f),(0.0f,1/4.0f),(0.0f,1/2.0f),(0.0f,3/4.0f),(1/2.0f,1/4.0f),(1/2.0f,1/2.0f),(1/2.0f,3/4.0f)}
void main() {
     if (u_selectMode > 0) {
    //     // 選択のIDが0より大きければIDで描画する
    //      float c = f_dropId / 255.0;
        
    //      out_color = vec4(c,c,c,1.0);
         
    //  } else {
    //     // 描画色を代入
    //    if (f_dropId == u_selectDrop){
       out_color = vec4(texture(u_texture, f_texPos).rgb, 1.0);
     }else{
       out_color = vec4(texture(u_texture, f_texPos).rgb*0.3, 1.0);
    //    }   
     }
}