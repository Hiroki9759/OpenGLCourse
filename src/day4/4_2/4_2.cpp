#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// ディレクトリの設定ファイル
#include <common.h>

static int WIN_WIDTH = 500;                       // ウィンドウの幅
static int WIN_HEIGHT = 500;                       // ウィンドウの高さ
static const char *WIN_TITLE = "OpenGL Course";     // ウィンドウのタイトル

static const double PI = 4.0 * std::atan(1.0);

// シェーダファイル
static std::string VERT_SHADER_FILE = std::string(SHADER_DIRECTORY) + "render.vert";
static std::string FRAG_SHADER_FILE = std::string(SHADER_DIRECTORY) + "render.frag";

static std::string TEX_FILE = std::string(DATA_DIRECTORY) + "dice.png";


// 頂点オブジェクト
struct Vertex {
	Vertex(const glm::vec3 &position_, const glm::vec2 &color_)
	    : position(position_)
		, color(color_){
	}

	glm::vec3 position;
	glm::vec2 color;
};

static const glm::vec3 positions[8] = {
	glm::vec3(-1.0f, -1.0f, -1.0f),
	glm::vec3(1.0f, -1.0f, -1.0f),
	glm::vec3(-1.0f,  1.0f, -1.0f),
	glm::vec3(-1.0f, -1.0f,  1.0f),
	glm::vec3(1.0f,  1.0f, -1.0f),
	glm::vec3(-1.0f,  1.0f,  1.0f),
	glm::vec3(1.0f, -1.0f,  1.0f),
	glm::vec3(1.0f,  1.0f,  1.0f)
};

static const unsigned int faces[12][3] = {
	{ 1, 6, 7 }, { 1, 7, 4 },
	{ 2, 5, 7 }, { 2, 7, 4 },
	{ 3, 5, 7 }, { 3, 7, 6 },
	{ 0, 1, 4 }, { 0, 4, 2 },
	{ 0, 1, 6 }, { 0, 6, 3 },
	{ 0, 2, 5 }, { 0, 5, 3 }
};

float dx = 1 / 4.0f, dy = 1 / 3.0f;
static const glm::vec2 tex_pos[6] = {
		glm::vec2(1 / 4.0f,  0.0f), // 1
		glm::vec2(0.0f,  1 / 3.0f), // 2
		glm::vec2(1 / 4.0f,  1 / 3.0f), // 3
		glm::vec2(3 / 4.0f,  1 / 3.0f), // 4
		glm::vec2(2 / 4.0f,  1 / 3.0f), // 5
		glm::vec2(1 / 4.0f,  2 / 3.0f), // 6
};

static const glm::vec2 tex_offset[6] = {
	glm::vec2(0.0, 0.0), glm::vec2(dx, 0.0),glm::vec2(dx, dy),
	glm::vec2(0.0, 0.0), glm::vec2(dx, dy), glm::vec2(0.0, dy)
};

// バッファを参照する番号
GLuint vaoId;
GLuint vertexBufferId;
GLuint indexBufferId;
GLuint textureId;

// シェーダを参照する番号
GLuint programId;

// 立方体の回転角度
static float theta = 0.0f;
// オブジェクトを選択するためのID
bool selectMode = false;
int selectsurface = 1;
 
// VAOの初期化
void initVAO() {
	// Vertex配列の作成
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	int idx = 0;
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 3; j++) {
			Vertex v(positions[faces[i * 2 + 0][j]], tex_pos[i] + tex_offset[j]);
			vertices.push_back(v);
			indices.push_back(idx++);
		}

		for (int j = 0; j < 3; j++) {
			Vertex v(positions[faces[i * 2 + 1][j]], tex_pos[i] + tex_offset[j + 3]);
			vertices.push_back(v);
			indices.push_back(idx++);
		}
	}

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
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

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
	reader.seekg(0, std::ios::end);                      // ファイル読み取り位置を終端に移動
	code.reserve(reader.tellg());                        // コードを格納する変数の大きさを予約 (文字列のサイズは変化しない)
	reader.seekg(0, std::ios::beg);                      // ファイルの読み取り位置を先頭に移動
	code.assign(std::istreambuf_iterator<char>(reader),
		std::istreambuf_iterator<char>());       // 先頭からファイルサイズ分を読んでコードの変数に格納

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
	if (linkState == GL_FALSE) {
		// リンクに失敗したらエラーメッセージを表示して終了
		fprintf(stderr, "Failed to link shaders!\n");

		// エラーメッセージの長さを取得する
		GLint logLength;
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);
		if (logLength > 0) {
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

	if (!bytes) {
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
	//glfwSwapInterval(1);
}

// OpenGLの描画関数
void paintGL() {
	// 背景色の描画
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 座標の変換
	glm::mat4 projMat = glm::perspective(45.0f,
		(float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 1000.0f);

	glm::mat4 viewMat = glm::lookAt(glm::vec3(3.0f, 4.0f, 5.0f),   // 視点の位置
		glm::vec3(0.0f, 0.0f, 0.0f),   // 見ている先
		glm::vec3(0.0f, 1.0f, 0.0f));  // 視界の上方向

	

	// シェーダの有効化
	glUseProgram(programId);
    // VAOの有効化
    glBindVertexArray(vaoId);
	glm::mat4 modelMat;
	modelMat = glm::rotate(modelMat, theta, glm::vec3(0.0f, 1.0f, 0.0f));
	modelMat = glm::rotate(modelMat, 2 * theta, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 mvpMat = projMat * viewMat * modelMat;
	// Uniform変数の転送
	GLuint LocId = glGetUniformLocation(programId, "u_mvpMat");
	glUniformMatrix4fv(LocId, 1, GL_FALSE, glm::value_ptr(mvpMat));
	// Uniform変数の転送
    GLuint uid;
              uid = glGetUniformLocation(programId, "u_mvpMat");
              glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(mvpMat));
              if (selectMode) {
                            uid = glGetUniformLocation(programId, "u_surfaceID");
                            glUniform1i(uid, selectMode ? 1 : -1);
 
                            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
 
                            uid = glGetUniformLocation(programId, "u_surfaceID");
                            glUniform1i(uid, selectMode ? 2 : -1);
 
                            glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
 
                            uid = glGetUniformLocation(programId, "u_surfaceID");
                            glUniform1i(uid, selectMode ? 3 : -1);
 
                            glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
 
                            uid = glGetUniformLocation(programId, "u_surfaceID");
                            glUniform1i(uid, selectMode ? 4 : -1);
 
                            glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);
 
                            uid = glGetUniformLocation(programId, "u_surfaceID");
                            glUniform1i(uid, selectMode ? 5 : -1);
 
                            glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);
 
                            uid = glGetUniformLocation(programId, "u_surfaceID");
                            glUniform1i(uid, selectMode ? 6 : -1);
 
                            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
              }
              else {
                            uid = glGetUniformLocation(programId, "u_surfaceID");
                            glUniform1i(uid, -1);
              }
 
              if (selectsurface == 1) {
                            uid = glGetUniformLocation(programId, "u_objectID");
                            glUniform1i(uid, 1);
 
                            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
 
                            uid = glGetUniformLocation(programId, "u_objectID");
                            glUniform1i(uid, -1);
 
                            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
              }
              else if (selectsurface == 2) {
                            uid = glGetUniformLocation(programId, "u_objectID");
                            glUniform1i(uid, -1);
 
                            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
 
                            uid = glGetUniformLocation(programId, "u_objectID");
                            glUniform1i(uid, 1);
 
                            glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
 
                            uid = glGetUniformLocation(programId, "u_objectID");
                            glUniform1i(uid, -1);
 
                            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
              }
              else if (selectsurface == 3) {
                            uid = glGetUniformLocation(programId, "u_objectID");
                            glUniform1i(uid, -1);
 
                            glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);
 
                            uid = glGetUniformLocation(programId, "u_objectID");
                            glUniform1i(uid, 1);
 
                            glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
 
                            uid = glGetUniformLocation(programId, "u_objectID");
                            glUniform1i(uid, -1);
 
                            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
              }
              else if (selectsurface == 4) {
                            uid = glGetUniformLocation(programId, "u_objectID");
                            glUniform1i(uid, -1);
 
                            glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
 
                            uid = glGetUniformLocation(programId, "u_objectID");
                            glUniform1i(uid, 1);
 
                            glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);
 
                            uid = glGetUniformLocation(programId, "u_objectID");
                            glUniform1i(uid, -1);
 
                            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
              }
              else if (selectsurface == 5) {
                            uid = glGetUniformLocation(programId, "u_objectID");
                            glUniform1i(uid, -1);
 
                            glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);
 
                            uid = glGetUniformLocation(programId, "u_objectID");
                            glUniform1i(uid, 1);
 
                            glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);
 
                            uid = glGetUniformLocation(programId, "u_objectID");
                            glUniform1i(uid, -1);
 
                            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
              }
              else if (selectsurface == 6) {
                            uid = glGetUniformLocation(programId, "u_objectID");
                            glUniform1i(uid, -1);
 
                            glDrawElements(GL_TRIANGLES, 30, GL_UNSIGNED_INT, 0);
 
                            uid = glGetUniformLocation(programId, "u_objectID");
                            glUniform1i(uid, 1);
 
                            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
              }
              else {
                            uid = glGetUniformLocation(programId, "u_objectID");
                            glUniform1i(uid, -1);
 
                            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
              }

    // テクスチャの有効化とシェーダへの転送
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);
	LocId = glGetUniformLocation(programId, "u_texture");
	glUniform1i(LocId, 0);
	
	
	// VAOの無効化
	glBindVertexArray(0);

	// シェーダの無効化
	glUseProgram(0);

	// テクスチャの無効化
 	glBindTexture(GL_TEXTURE_2D, 0);
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
void mouseEvent(GLFWwindow *window, int button, int action, int mods) {
    // クリックされた位置を取得
    double px, py;
    glfwGetCursorPos(window, &px, &py);
 
              if (action == GLFW_PRESS) {
                            // クリックされた位置を取得
                            double px, py;
                            glfwGetCursorPos(window, &px, &py);
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
                            unsigned char byte[4];
                            glReadPixels(cx * pixelSize, (WIN_HEIGHT - cy - 1)*pixelSize, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &byte);
                            printf("Mouse position: %d %d\n", cx, cy);
                            printf("Select surface %d\n", (int)byte[0]);
                            selectsurface = byte[0];
              }
}
// アニメーションのためのアップデート
void animate() {
	theta += 2.0f * PI / 360.0f;  // 10分の1回転
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

	// メインループ
	while (glfwWindowShouldClose(window) == GL_FALSE) {
		// 描画
		paintGL();

		// アニメーション
		animate();

		// 描画用バッファの切り替え
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}
