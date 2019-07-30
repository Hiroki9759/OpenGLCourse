#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <ctime>
#include <random>
#include <vector>
//乱数を取得するために現在の時刻を取得する
#include<time.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// ディレクトリの設定ファイル
#include "common.h"

static int WIN_WIDTH   = 500;                       // ウィンドウの幅
static int WIN_HEIGHT  = 500;                       // ウィンドウの高さ
static const char *WIN_TITLE = "OpenGL Course";     // ウィンドウのタイトル
static float theta = 0.0f;
static const double fps = 120.0;                     // FPS
// シェーダファイル
static std::string VERT_SHADER_FILE = std::string(SHADER_DIRECTORY) + "render.vert";
static std::string FRAG_SHADER_FILE = std::string(SHADER_DIRECTORY) + "render.frag";
//オブジェクトファイル
static std::string TEX_FILE = std::string(DATA_DIRECTORY) + "puzzledrop.png";

// 頂点オブジェクト
struct Vertex {
    Vertex(const glm::vec3 &position_, const glm::vec2 &tex_)
        : position(position_)
        , tex(tex_) {
    }
    glm::vec3 position;
    glm::vec2 tex;
};

static const  glm::vec3 positions[4]={
    glm::vec3(-1.0f,-1.0f,0.0f),
    glm::vec3(-0.75f,-1.0f,0.0f),
    glm::vec3(-1.0f,-0.75f,0.0f),
    glm::vec3(-0.75,-0.75f,0.0f)
};
static const unsigned int drops[2][3] = {
     { 1, 0, 2 }, { 1, 3, 2 }
};
static const glm::vec2 type_diff[8]={
    glm::vec2(-0.5f,0.77f),glm::vec2(-0.5f,0.53f),glm::vec2(-0.5f,0.25f),glm::vec2(-0.5f,0.0f),
    glm::vec2(0.0f,0.75f),glm::vec2(0.0f,0.5f),glm::vec2(0.0f,0.25f),glm::vec2(0.0f,0.0f)
};

float dx = 1.0f / 2.0f, dy = 1.0f / 4.0f;
static const  glm::vec2 tex_pos =  glm::vec2(0.5f,  0.0f);


static const glm::vec2 tex_offset[6] =
 {
	glm::vec2(0.0f,0.0f), glm::vec2(dx, 0.0f),glm::vec2(dx, -dy),
        glm::vec2(0.0f, 0.0f),glm::vec2(0.0f,-dy),glm::vec2(dx,-dy)
};

// バッファを参照する番号
GLuint vaoId;
GLuint vertexBufferId;
GLuint indexBufferId;

// シェーダを参照する番号
GLuint programId;
GLuint textureId;
// Arcballコントロールのための変数
bool isDragging = false;
enum ArcballMode {
    ARCBALL_MODE_NONE = 0x00,
    ARCBALL_MODE_TRANSLATE = 0x01,
    ARCBALL_MODE_ROTATE = 0x02,
    ARCBALL_MODE_SCALE = 0x04
};
int arcballMode = ARCBALL_MODE_NONE;
int droptypenumber[64];

int selectObject = 0;
bool selectMode = false;

glm::vec3 gravity;
glm::ivec2 oldPos;
glm::ivec2 newPos;


struct Drop{
    glm::mat4 modelMat,acTransMat;
    static int dropnumber;
    int dropId;
    int droptype;

    Drop(){
        dropnumber++;
        dropId = dropnumber;
    }

    void initialize() {
        
        // その他の行列の初期化
        modelMat = glm::mat4(1.0);
        acTransMat = glm::mat4(1.0);

    }
    void draw(){
        glm::mat4 mvpMat = modelMat * acTransMat;

        // Uniform変数の転送
        GLuint uid;
        uid = glGetUniformLocation(programId, "u_mvpMat");
        glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(mvpMat));
        uid = glGetUniformLocation(programId, "u_selectID");
        glUniform1i(uid, selectMode ? dropId : -1);
        uid = glGetUniformLocation(programId, "u_texcoord");
        glUniform2fv(uid,1,glm::value_ptr(type_diff[droptype])); 

        glDrawElements(GL_TRIANGLES, 48, GL_UNSIGNED_INT, 0);
    }
};
int Drop::dropnumber = 0;
Drop  drop[64];


// VAOの初期化
void initVAO() {
    // Vertex配列の作成
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    int idx = 0;
    gravity = glm::vec3(0.0f, 0.0f, 0.0f);
    for (int k = 0; k < 2; k++) {
        for (int j = 0; j < 3; j++){
            Vertex v(positions[drops[k][j]], tex_pos + tex_offset[j+k*3]);
            vertices.push_back(v);
            indices.push_back(idx++);
            gravity += v.position;
        }
    }
    gravity /= indices.size();
  
    // VAOの作成
    glGenVertexArrays(1, &vaoId);
    glBindVertexArray(vaoId);
  
    // 頂点バッファの作成
    glGenBuffers(1, &vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
  
    // 頂点バッファの有効化
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex));
  
    // 頂点番号バッファの作成
    glGenBuffers(1, &indexBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);
   
    // VAOをOFFにしておく
    glBindVertexArray(0);
}

GLuint compileShader(const std::string &filename, GLuint type) {
   
    // シェーダの作成
    GLuint shaderId = glCreateShader(type);
  
    // ファイルの読み込み
    std::ifstream reader;
    size_t codeSize;
    std::string code;
   
    // ファイルを開く
    reader.open(filename.c_str(), std::ios::in);
    if (!reader.is_open()) {
        // ファイルを開けなかったらエラーを出して終了
        fprintf(stderr, "Failed to load a shader: %s\n", VERT_SHADER_FILE.c_str());
        exit(1);
    }
  
    // ファイルをすべて読んで変数に格納 (やや難)
    reader.seekg(0, std::ios::end);             // ファイル読み取り位置を終端に移動
    codeSize = reader.tellg();                  // 現在の箇所(=終端)の位置がファイルサイズ
    code.resize(codeSize);                      // コードを格納する変数の大きさを設定
    reader.seekg(0);                            // ファイルの読み取り位置を先頭に移動
    reader.read(&code[0], codeSize);            // 先頭からファイルサイズ分を読んでコードの変数に格納
   
    // ファイルを閉じる
    reader.close();
   
    // コードのコンパイル
    const char *codeChars = code.c_str();
    glShaderSource(shaderId, 1, &codeChars, NULL);
    glCompileShader(shaderId);
 
   // コンパイルの成否を判定する
    GLint compileStatus;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE) {
    
        // コンパイルが失敗したらエラーメッセージとソースコードを表示して終了
        fprintf(stderr, "Failed to compile a shader!\n");
    
        // エラーメッセージの長さを取得する
        GLint logLength;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
   
            // エラーメッセージを取得する
            GLsizei length;
            std::string errMsg;
            errMsg.resize(logLength);
            glGetShaderInfoLog(shaderId, logLength, &length, &errMsg[0]);

            // エラーメッセージとソースコードの出力
            fprintf(stderr, "[ ERROR ] %s\n", errMsg.c_str());
            fprintf(stderr, "%s\n", code.c_str());
        }
        exit(1);
    }
    return shaderId;
}

GLuint buildShaderProgram(const std::string &vShaderFile, const std::string &fShaderFile) {
   
    // シェーダの作成
    GLuint vertShaderId = compileShader(vShaderFile, GL_VERTEX_SHADER);
    GLuint fragShaderId = compileShader(fShaderFile, GL_FRAGMENT_SHADER);
   
    // シェーダプログラムのリンク
    GLuint programId = glCreateProgram();
    glAttachShader(programId, vertShaderId);
    glAttachShader(programId, fragShaderId);
    glLinkProgram(programId);
  
    // リンクの成否を判定する
    GLint linkState;
    glGetProgramiv(programId, GL_LINK_STATUS, &linkState);
    if (linkState == GL_FALSE){
     
        // リンクに失敗したらエラーメッセージを表示して終了
        fprintf(stderr, "Failed to link shaders!\n");

        // エラーメッセージの長さを取得する
        GLint logLength;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0){
          
            // エラーメッセージを取得する
            GLsizei length;
            std::string errMsg;
            errMsg.resize(logLength);
            glGetProgramInfoLog(programId, logLength, &length, &errMsg[0]);

            // エラーメッセージを出力する
            fprintf(stderr, "[ ERROR ] %s\n", errMsg.c_str());
        }
        exit(1);
    }
  
    // シェーダを無効化した後にIDを返す
    glUseProgram(0);
    return programId;
}

// シェーダの初期化
void initShaders() {
    programId = buildShaderProgram(VERT_SHADER_FILE, FRAG_SHADER_FILE);
}

void initTexture() {
	// テクスチャの設定
	int texWidth, texHeight, channels;
	unsigned char *bytes = stbi_load(TEX_FILE.c_str(), &texWidth, &texHeight, &channels, STBI_rgb_alpha);
	if (!bytes) 
    {
		fprintf(stderr, "Failed to load image file: %s\n", TEX_FILE.c_str());
		exit(1);
	}
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(bytes);
}

// OpenGLの初期化関数
void initializeGL() {
    // 深度テストの有効化
    glEnable(GL_DEPTH_TEST);
   
    // 背景色の設定 (黒)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
   
    // VAOの初期化
    initVAO();
   
    // シェーダの用意
    initShaders();
  
    //テクスチャの用意
    initTexture();
   
    //ドロップの初期配置
    for(int i = 0;i<64;i++){
       srand((unsigned int)time(NULL));
        int l = rand()%7;
        drop[i].initialize();
        drop[i].modelMat = glm::translate(drop[i].modelMat, glm::vec3((i%8)*0.25f, (i/8)*0.25f, 0.0f));
        drop[i].droptype = l; 
        droptypenumber[i] = l;
    }
}
//各処理の後の描画
void paintGL() {
    
    // 背景色の描画
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   
    // シェーダの有効化
    glUseProgram(programId);
   
    // VAOの有効化
    glBindVertexArray(vaoId);
   
    // テクスチャの有効化とシェーダへの転送
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);
   
    //立方体を描画
    for(int j = 0;j<64;j++){
        drop[j].droptype = droptypenumber[j];  
        drop[j].draw();
    }

    // VAOの無効化
    glBindVertexArray(0);

    // シェーダの無効化
    glUseProgram(0);
    
    // テクスチャの無効化
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//ドロップの入れ替え
void dropchange(int cgnum){
    int save  = droptypenumber[selectObject];
    int save2 = droptypenumber[selectObject+cgnum];
    // std::cout << "before swap\n";
    // std::cout << "clicked: " << droptypenumber[selectObject] << std::endl;
    // std::cout  << "swapped: " << droptypenumber[selectObject+cgnum] << std::endl;
    droptypenumber[selectObject]=save2;
    droptypenumber[selectObject+cgnum]=save; 
    // std::cout << "after swap\n";
    // std::cout << "clicked: " << droptypenumber[selectObject] << std::endl;
    // std::cout  << "swapped: " << droptypenumber[selectObject+cgnum] << std::endl; 

    paintGL();
}

int n = 0;
//3つ以上揃ったら×にする
void match() {

    for(int i = 0;i<62;i++){

        if(((droptypenumber[i]==droptypenumber[i+1])&&(droptypenumber[i+1]==droptypenumber[i+2]))&&(droptypenumber[i+2]==droptypenumber[i+3])&&(droptypenumber[i+3]==droptypenumber[i+4])&&((i/8)==((i+4)/8))&&(i<60)){
            droptypenumber[i] = 8;
            droptypenumber[i+1] = 8;
            droptypenumber[i+2] = 8; 
            droptypenumber[i+3] = 8; 
            droptypenumber[i+4] = 8;
            std::cout << "match5drop:" << i << "," << i+1  << "," << i+2<< "," << i+3 << "," << i+4 << std::endl; 
            n++;
        }else if(((droptypenumber[i]==droptypenumber[i+1])&&(droptypenumber[i+1]==droptypenumber[i+2]))&&(droptypenumber[i+2]==droptypenumber[i+3])&&((i/8)==((i+3)/8))&&(i<61)){
            droptypenumber[i] = 8;
            droptypenumber[i+1] = 8;
            droptypenumber[i+2] = 8; 
            droptypenumber[i+3] = 8; 
            std::cout << "match4drop:" << i << "," << i+1  << "," << i+2 << "," << i+3 << std::endl; 
            n++;
        }else if(((droptypenumber[i]==droptypenumber[i+1])&&(droptypenumber[i+1]==droptypenumber[i+2]))&&((i/8)==((i+2)/8))){
            droptypenumber[i] = 8;
            droptypenumber[i+1] = 8;
            droptypenumber[i+2] = 8; 
            std::cout << "match3drop:" << i << "," << i+1  << "," << i+2 << std::endl; 
            n++;
        }
    }
    for(int i =0;i<48;i++){
        if ((droptypenumber[i]==droptypenumber[i+8])&&(droptypenumber[i+8]==droptypenumber[i+16])&&(droptypenumber[i+16]==droptypenumber[i+24])&&(droptypenumber[i+24]==droptypenumber[i+32])&&(i<32)){
            droptypenumber[i] = 8;
            droptypenumber[i+8] = 8;
            droptypenumber[i+16] = 8;
            droptypenumber[i+24] = 8; 
            droptypenumber[i+32] = 8;  
            std::cout << "match5drop:" << i << "," << i+8  << "," << i+16<< "," << i+24 << "," << i+32 << std::endl; 
            n++;
        }else if ((droptypenumber[i]==droptypenumber[i+8])&&(droptypenumber[i+8]==droptypenumber[i+16])&&(droptypenumber[i+16]==droptypenumber[i+24])&&(i<40)){
            droptypenumber[i] = 8;
            droptypenumber[i+8] = 8;
            droptypenumber[i+16] = 8;
            droptypenumber[i+24] = 8; 
            std::cout << "match4drop:" << i << "," << i+8  << "," << i+16<< "," << i+24 << std::endl; 
         　  n++;
        }else if ((droptypenumber[i]==droptypenumber[i+8])&&(droptypenumber[i+8]==droptypenumber[i+16])){
            droptypenumber[i] = 8;
            droptypenumber[i+8] = 8;
            droptypenumber[i+16] = 8;
            std::cout << "match3drop:" << i << "," << i+8  << "," << i+16 << std::endl; 
            n++;
        }
    }
    paintGL();
}

//落とし
void falldrop()
{   
    for(int i = 55;i >=0;i--)
    {
        //消えた場所にドロップを落とす
        if((droptypenumber[i+8] != 8 )&& (droptypenumber[i] == 8))
        {
            // std::cout << "beforefalldropup  :" << droptypenumber[i+8] << "," << i+8 <<std::endl; 
            // std::cout << "beforefalldropdown:" << droptypenumber[i] << "," << i <<std::endl; 
           
            int save = droptypenumber[i];
            droptypenumber[i] = droptypenumber[i+8];
            droptypenumber[i+8]=8;
            // std::cout << "afterfalldropup   :" << droptypenumber[i+8] << "," << i+8 <<std::endl; 
            // std::cout << "afterfalldropdown :" << droptypenumber[i] << "," << i <<std::endl; 
           
            for(int i=0;i<64;i++){
                drop[i].draw();
            } 
        }
    }
    //一番上でドロップを補充する
    for (int k = 56; k < 64; k++)
    {
        if (droptypenumber[k]== 8)
        {
            droptypenumber[k] = rand()%7;
        }
    }
    paintGL();
}

void resizeGL(GLFWwindow *window, int width, int height) {
   
    // ユーザ管理のウィンドウサイズを変更
    WIN_WIDTH = width;
    WIN_HEIGHT = height;

    // GLFW管理のウィンドウサイズを変更
    glfwSetWindowSize(window, WIN_WIDTH, WIN_HEIGHT);

    // 実際のウィンドウサイズ (ピクセル数) を取得
    int renderBufferWidth, renderBufferHeight;
    glfwGetFramebufferSize(window, &renderBufferWidth, &renderBufferHeight);

    // ビューポート変換の更新
    glViewport(0, 0, renderBufferWidth, renderBufferHeight);

}

unsigned char byte[4];
void mouseEvent(GLFWwindow *window, int button, int action, int mods) {
   
    // クリックされた位置を取得
    double px, py;
    glfwGetCursorPos(window, &px, &py);

    if (action == GLFW_PRESS){
        const int cx = (int)px;
        const int cy = (int)py;

        // 選択モードでの描画
        selectMode = true;
         paintGL();
        selectMode = false;
        
        // ピクセルの大きさの計算 (Macの場合には必要)
        int renderBufferWidth, renderBufferHeight;
        glfwGetFramebufferSize(window, &renderBufferWidth, &renderBufferHeight);
        int pixelSize = std::max(renderBufferWidth / WIN_WIDTH, renderBufferHeight / WIN_HEIGHT);
        
        // より適切なやり方
        glReadPixels(cx*pixelSize, (WIN_HEIGHT - cy - 1)*pixelSize, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &byte);
        printf("Mouse position: %d %d\n", cx, cy);
        selectObject = (int)byte[0]-1;
        printf("SelectDropnumber %d\n", selectObject);
        printf("selectDroptype: %d\n",droptypenumber[selectObject]);
    }
    std::cout << "十字キーを押すと隣のドロップと入れ替わります"<<std::endl; 
}

void updateTranslate() {

    // オブジェクト重心のスクリーン座標を求める
     glm::vec4 gravityScreenSpace = (  drop[selectObject].modelMat) * glm::vec4(gravity.x, gravity.y, gravity.z, 1.0f);
    gravityScreenSpace /= gravityScreenSpace.w;

    // スクリーン座標系における移動量
     glm::vec4 newPosScreenSpace(2.0 * newPos.x / WIN_WIDTH, -2.0 * newPos.y / WIN_HEIGHT, gravityScreenSpace.z, 1.0f);
     glm::vec4 oldPosScreenSpace(2.0 * oldPos.x / WIN_WIDTH, -2.0 * oldPos.y / WIN_HEIGHT, gravityScreenSpace.z, 1.0f);

    // スクリーン座標の情報をオブジェクト座標に変換する行列
    const glm::mat4 s2oMat = glm::inverse(  drop[selectObject].modelMat);

    // スクリーン空間の座標をオブジェクト空間に変換
    glm::vec4 newPosObjSpace = s2oMat* newPosScreenSpace;
    glm::vec4 oldPosObjSpace = s2oMat * oldPosScreenSpace;
    newPosObjSpace /= newPosObjSpace.w;
    oldPosObjSpace /= oldPosObjSpace.w;
    // オブジェクト座標系での移動量
    const glm::vec3 transObjSpace= glm::vec3(newPosObjSpace - oldPosObjSpace);

    // オブジェクト空間での平行移動
    drop[selectObject].acTransMat = glm::translate(drop[selectObject].acTransMat, transObjSpace);
}


void keyboardEvent(GLFWwindow *window, int key, int scancode, int action, int mods) {
   
    // キーボードの状態と押されたキーを表示する
   // printf("Keyboard: %s\n", action == GLFW_PRESS ? "Press" : "Release");
    if (action == GLFW_PRESS){
        if (key == GLFW_KEY_LEFT){
            printf("Key: ←\n" );
            dropchange(-1);
        }else if(key == GLFW_KEY_RIGHT){
            printf("Key: →\n" );
            dropchange(1);
        }else if(key == GLFW_KEY_UP){
            printf("Key: ↑\n" );
            dropchange(8);
        }else if(key == GLFW_KEY_DOWN){
            printf("Key: ↓\n" );
            dropchange(-8);
        } 
        for(int j = 0;j<8;j++)
        {   
            match();

            for (int i = 0; i < 8; i++)
            {
                falldrop();
            }
        }
        std::cout << "現在の総combo数は:" <<  n <<"です。"<< std::endl;
        std::cout << "動かすドロップを選択してください。" << std::endl; 
        if(n>=20){
           std::cout << "ゲームクリアです！お疲れ様でした。" << std::endl; 
        }
    }
}

int main(int argc, char **argv) {
    
    // OpenGLを初期化する
    if (glfwInit() == GL_FALSE) {
        fprintf(stderr, "Initialization failed!\n");
        return 1;
    }

    // OpenGLのバージョン設定 (Macの場合には必ず必要)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Windowの作成
    GLFWwindow *window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE,
                                          NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Window creation failed!");
        glfwTerminate();
        return 1;
    }

    // OpenGLの描画対象にWindowを追加
    glfwMakeContextCurrent(window);

    // マウスのイベントを処理する関数を登録
    glfwSetMouseButtonCallback(window, mouseEvent);
  　// キーボードのイベントを処理する関数を登録
    glfwSetKeyCallback(window, keyboardEvent);

    // OpenGL 3.x/4.xの関数をロードする (glfwMakeContextCurrentの後でないといけない)
    const int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        fprintf(stderr, "Failed to load OpenGL 3.x/4.x libraries!\n");
        return 1;
    }

    // バージョンを出力する
    printf("Load OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    // ウィンドウのリサイズを扱う関数の登録
    glfwSetWindowSizeCallback(window, resizeGL);

    // OpenGLを初期化
    initializeGL();
    match();
    
   for(int i = 0;i<8;i++){
        match();
        for(int j = 0; j<8; j++){
            falldrop(); 
        }
    }
    paintGL();
    n = 0;
    if((droptypenumber[57]!=8)&&(droptypenumber[58]!=8)&&(droptypenumber[59]!=8)&&(droptypenumber[60]!=8)&&(droptypenumber[61]!=8)&&(droptypenumber[62]!=8)&&(droptypenumber[63]!=8)&&(droptypenumber[64]!=8)){
        std::cout << "動かすドロップを選択してください。\n 20コンボでクリアです。" << std::endl;

    }
    glfwSwapBuffers(window);
    glfwPollEvents();  
    double prevTime = glfwGetTime();

    // メインループ
    while (glfwWindowShouldClose(window) == GL_FALSE){
        paintGL();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
