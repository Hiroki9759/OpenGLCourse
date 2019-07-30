// 2018.11.8 version

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glfw3.lib")


#define GLFW_INCLUDE_GLU
#define _USE_MATH_DEFINES

#include <iostream>
#include <cmath>

#include <GLFW/glfw3.h>

void drawOpenGL(void);
void drawCube(void);
void Ground(void);

int WindowWidth = 640;      //��������E�B���h�E�̕�
int WindowHeight = 480;     //��������E�B���h�E�̍���
char WindowTitle[] = "Motion of Robot Arm";  //�E�B���h�E�̃^�C�g��

//static double theta = 0.0f;

// eight vertices of each robot arm (2�̃��{�b�g�A�[���͓������_�����B�����Z�ʑ�)
static const float positions[8][3] = {
	{  0.0f, -0.01f,  0.0f },
	{  0.3f, -0.01f,  0.0f },
	{  0.0f,  0.01f,  0.0f },
	{  0.0f, -0.01f,  0.01f },
	{  0.3f,  0.01f,  0.0f },
	{  0.0f,  0.01f,  0.01f },
	{  0.3f, -0.01f,  0.01f },
	{  0.3f,  0.01f,  0.01f }
};

// Color
static const float colors[2][3] = {
	{ 1.0f, 1.0f, 1.0f },  
	{ 1.0f, 0.0f, 0.0f }
};

// triangle definition of earch robot arm (�ꉞ�A�Z�ʑ�)
static const unsigned int indices[12][3] = {
	{ 1, 6, 7 }, { 1, 7, 4 },
	{ 2, 5, 7 }, { 2, 7, 4 },
	{ 3, 5, 7 }, { 3, 7, 6 },
	{ 0, 1, 4 }, { 0, 4, 2 },
	{ 0, 1, 6 }, { 0, 6, 3 },
	{ 0, 2, 5 }, { 0, 5, 3 }
};

//--------------- Robot Arm --------------------------
const double L1 = 0.3;
const double L2 = 0.3;
const double Lg1 = 0.15;
const double Lg2 = 0.15;
const double m1 = 0.5;
const double m2 = 0.5;
const double I1 = 5.4e-3;
const double I2 = 5.4e-3;
const double mL = 5.0;
const double xvh = 1.0;
const double yvh = 1.0;
const double g = 9.806199;
const double pi = 3.141592;
const double h = 0.001;

//--------- ���l�v�Z���ʂ̕ۑ��� ----------
double t, th1[801], th2[801], thv1[801], thv2[801];
double X1[801], Y1[801], X2[801], Y2[801];

// ���l�v�Z���ʂ̎��Ԍo�߁@(0����800�܂ł�k�Ԗ�)
int k;

// ���[�U��`��OpenGL�`��
void drawOpenGL() {
	double X1, Y1, angle1, angle2;
		
	glClear(GL_COLOR_BUFFER_BIT); // �w�i�F�̕`��
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // �w�i�F�Ɛ[�x�l�̃N���A
	glViewport(0, 0, WindowWidth, WindowHeight); // �r���[�|�[�g�ϊ��̎w��

	// ���W�ϊ�
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (double)WindowWidth / (double)WindowHeight, 0.1f, 1000.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(0.0f, 0.1f, 2.0f,   // ���_�̈ʒu
		0.0f, 0.0f, 0.0f,         // ���Ă����
		0.0f, 0.0f, 1.0f);        // ���E�̏����

	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	Ground(); // ��n�̕`��

	angle1 = th1[k] * 180.0 / M_PI - 180.0; //��r
	angle2 = th2[k] * 180.0 / M_PI;         //�O�r

	X1 = 0.3;
	Y1 = 0.0;

	glRotatef(angle1, 0.0f, 0.0f, 1.0f);
	
	drawCube();  // �Z�ʑ�(��r�A�[��)�̕`��

	glTranslated(X1, Y1, 0);
	glRotatef(angle2, 0.0f, 0.0f, 1.0f);
	drawCube();  // �Z�ʑ�(�O�r�A�[��)�̕`��

	//animate();
	if(k<=800) k++;
	else k = 0;
}



void drawCube() {
	glBegin(GL_TRIANGLES);
	glColor3fv(colors[1]); // red color
	for (int face = 0; face < 6; face++) {
		for (int i = 0; i < 3; i++) {
			glVertex3fv(positions[indices[face * 2 + 0][i]]);
		}

		for (int i = 0; i < 3; i++) {
			glVertex3fv(positions[indices[face * 2 + 1][i]]);
		}
	}
	glEnd();
}



void Ground(void) {
	double ground_max_x = 0.5;
	double ground_max_y = 0.5;
	glClearColor(0.8f, 0.8f, 0.8f, 0.0f);  // ��n�̐F gray color
	glColor3fv(colors[0]); // �}�X�ڂ̐F�@white color

	glBegin(GL_LINES);
	for (double ly = -ground_max_y; ly <= ground_max_y; ly += 0.1) {
		glVertex3d(-ground_max_x, ly, 0);
		glVertex3d(ground_max_x, ly, 0);
	}
	for (double lx = -ground_max_x; lx <= ground_max_x; lx += 0.1) {
		glVertex3d(lx, ground_max_y, 0);
		glVertex3d(lx, -ground_max_y, 0);
	}
	glEnd();
}


int main() {
	int i, n;
	double A1, A2, A3, B1, B2;
	double z1, z2, w1, w2;
	double z21, z22, w21, w22;
	double z31, z32, w31, w32;
	double z41, z42, w41, w42;
	double Xa, Ya, za1, za2, thi1, thi2;
	double a11, a12, a22, u1, u2, LL;
	double k11, k12, k21, k22, k31, k32, k41, k42;
	double L11, L12, L21, L22, L31, L32, L41, L42;
	double tau1, tau2;   //���[�^�g���N
	double tauL1, tauL2; //���׃g���N
	double FL, FLx, FLy; //�O��
	double Xp, Yp; // ���݂̈ʒu
	double Xv, Yv; // ���݂̑��x

	A1 = m1 * Lg1*Lg1 + I1 + m2 * L1*L1 + mL * L1*L1;
	A2 = I2 + m2 * Lg2*Lg2 + mL * L2*L2;
	A3 = (m2*Lg2 + mL * L2)*L1;
	B1 = (m1*Lg1 + m2 * L1 + mL * L1)*g;
	B2 = (m2*Lg2 + mL * L2)*g;

	// ----- �����l�ݒ� -----
	thi1 = 90.0*pi / 180.0;
	thi2 = -90.0*pi / 180.0;

	t = 0.0;
	z1 = thi1;
	z2 = thi2;
	w1 = 0;
	w2 = 0;
	Yp = L1;
	Xv = 0;
	Yv = 0;

	// 800 ms�Ԃ̃��[�v
	for (i = 0; i <= 800; i++) {
		t = t + h;

		Yp = L1 * sin(z1) + L2 * sin(z1 + z2);
		Xv = (-L1 * sin(z1) - L2 * sin(z1 + z2))*w1
			+ (-L2 * sin(z1 + z2))*w2;
		Yv = (L1*cos(z1) + L2 * cos(z1 + z2))*w1
			+ (L2*cos(z1 + z2))*w2;

		// ----- �����@�O�́@���� -----
		if(Xp > 0/4){
			FLx = -(Xp -0.4) * 100000.0 - Xv*1000.0;
			FLy = 0.0;
		}
		else{
			FLx = 0.0;
			FLy = 0.0;
		}
		

		// ----- ����� -----
		tau1 = 0.0;
		tau2 = 0.0;

		// ----- �����Q�E�N�b�^�@�@��1�i�K -----
		k11 = h * (w1);
		k12 = h * (w2);
		LL = A1 * A2 - A3 * A3*cos(z2)*cos(z2);
		a11 = A2 / LL;
		a12 = -(A2 + A3 * cos(z2)) / LL;
		a22 = (A1 + A2 + 2.0 * A3*cos(z2)) / LL;
		tauL1 = (-L1 * sin(z1) - L2 * sin(z1 + z2))*FLx
			+ (L1*cos(z1) + L2 * cos(z1 + z2))*FLy;
		tauL2 = (-L2 * sin(z1 + z2))*FLx
			+ (L2*cos(z1 + z2))*FLy;
		u1 = tau1 + tauL1
			+ A3 * (2.0*w1*w2 + w2 * w2)*sin(z2)
			- B1 * cos(z1) - B2 * cos(z1 + z2);
		u2 = tau2 + tauL2
			- A3 * w1*w1*sin(z2)
			- B2 * cos(z1 + z2);
		L11 = h * (a11*u1 + a12 * u2);
		L12 = h * (a12*u1 + a22 * u2);

		// ----- �����Q�E�N�b�^�@�@��2�i�K -----
		z21 = z1 + k11 / 2.0;
		z22 = z2 + k12 / 2.0;
		w21 = w1 + L11 / 2.0;
		w22 = w2 + L12 / 2.0;
		k21 = h * (w21);
		k22 = h * (w22);
		LL = A1 * A2
			- A3 * A3*cos(z22)*cos(z22);
		a11 = A2 / LL;
		a12 = -(A2 + A3 * cos(z22)) / LL;
		a22 = (A1 + A2 + 2.0 * A3*cos(z22)) / LL;
		tauL1 = (-L1 * sin(z21) - L2 * sin(z21 + z22))*FLx
			+ (L1*cos(z21) + L2 * cos(z21 + z22))*FLy;
		tauL2 = (-L2 * sin(z21 + z22))*FLx
			+ (L2*cos(z21 + z22))*FLy;
		u1 = tau1 + tauL1
			+ A3 * (2.0*w21*w22 + w22 * w22)*sin(z22)
			- B1 * cos(z21) - B2 * cos(z21 + z22);
		u2 = tau2 + tauL2
			- A3 * w21*w21*sin(z22)
			- B2 * cos(z21 + z22);
		L21 = h * (a11*u1 + a12 * u2);
		L22 = h * (a12*u1 + a22 * u2);

		// ----- �����Q�E�N�b�^�@�@��3�i�K -----
		z31 = z1 + k21 / 2.0;
		z32 = z2 + k22 / 2.0;
		w31 = w1 + L21 / 2.0;
		w32 = w2 + L22 / 2.0;
		k31 = h * w31;
		k32 = h * w32;
		LL = A1 * A2 - A3 * A3*cos(z32)*cos(z32);
		a11 = A2 / LL;
		a12 = -(A2 + A3 * cos(z32)) / LL;
		a22 = (A1 + A2 + 2.0 * A3*cos(z32)) / LL;
		tauL1 = (-L1 * sin(z31) - L2 * sin(z31 + z32))*FLx
			+ (L1*cos(z31) + L2 * cos(z31 + z32))*FLy;
		tauL2 = (-L2 * sin(z31 + z32))*FLx
			+ (L2*cos(z31 + z32))*FLy;
		u1 = tau1 + tauL1
			+ A3 * (2.0*w31*w32 + w32 * w32)*sin(z32)
			- B1 * cos(z31) - B2 * cos(z31 + z32);
		u2 = tau2 + tauL2
			- A3 * w31*w31*sin(z32)
			- B2 * cos(z31 + z32);
		L31 = h * (a11*u1 + a12 * u2);
		L32 = h * (a12*u1 + a22 * u2);

		// ----- �����Q�E�N�b�^�@�@��4�i�K -----
		z41 = z1 + k31;
		z42 = z2 + k32;
		w41 = w1 + L31;
		w42 = w2 + L32;
		k41 = h * w41;
		k42 = h * w42;
		LL = A1 * A2
			- A3 * A3*cos(z42)*cos(z42);
		a11 = A2 / LL;
		a12 = -(A2 + A3 * cos(z42)) / LL;
		a22 = (A1 + A2 + 2.0 * A3*cos(z42)) / LL;
		tauL1 = (-L1 * sin(z41) - L2 * sin(z41 + z42))*FLx
			+ (L1*cos(z41) + L2 * cos(z41 + z42))*FLy;
		tauL2 = (-L2 * sin(z41 + z42))*FLx
			+ (L2*cos(z41 + z42))*FLy;
		u1 = tau1 + tauL1
			+ A3 * (2.0*w41*w42 + w42 * w42)*sin(z42)
			- B1 * cos(z41) - B2 * cos(z41 + z42);
		u2 = tau2 + tauL2
			- A3 * w41*w41*sin(z42)
			- B2 * cos(z41 + z42);
		L41 = h * (a11*u1 + a12 * u2);
		L42 = h * (a12*u1 + a22 * u2);

		// ----- �����Q�E�N�b�^�@�@�S�i���� -----
		z1 = z1 + (k11 + 2.0*k21 + 2.0*k31 + k41) / 6.0;
		z2 = z2 + (k12 + 2.0*k22 + 2.0*k32 + k42) / 6.0;
		w1 = w1 + (L11 + 2.0*L21 + 2.0*L31 + L41) / 6.0;
		w2 = w2 + (L12 + 2.0*L22 + 2.0*L32 + L42) / 6.0;

		th1[i] = z1;  th2[i] = z2;
		thv1[i] = w1; thv2[i] = w2;
		X1[i] = L1 * cos(z1);
		Y1[i] = L1 * sin(z1);
		X2[i] = L1 * cos(z1) + L2 * cos(z1 + z2);
		Y2[i] = L1 * sin(z1) + L2 * sin(z1 + z2);

		if (i % 10 == 0) {
			printf("i=%3d z1=%7.2f z2=%7.2f   "
				, i, th1[i] * 180.0 / pi, th2[i] * 180.0 / pi);
			printf("X=%6.3f Y=%6.3f   ", X2[i], Y2[i]);
			printf("tu1=%7.3f tu2=%7.3f\n", tau1, tau2);
		}
	}

	// OpenGL�̏�����
	if (glfwInit() == GL_FALSE) {
		fprintf(stderr, "Initialization failed!\n");
		return 1;
	}

	GLFWwindow *window = glfwCreateWindow(WindowWidth, WindowHeight, WindowTitle,
		NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Window creation failed!");
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);  // OpenGL�̕`��Ώۂ�Window��ǉ�
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);  // �w�i�F�̐ݒ�
	
	double FPS = 1000.0, current = 0.0, last = 0.0, elapsed = 0.0; // �A�j���[�V�����̎��Ԑݒ�
	glfwSetTime(0.0); //�������Z�b�g

	// Open GL ���C�����[�v
	while (glfwWindowShouldClose(window) == GL_FALSE ) {

		current = glfwGetTime(); //���ݎ����̎擾
		elapsed = current - last;
		if (elapsed >= 1.0 / (FPS*0.2)) { // �Đ����x�@1/5�{ (�x�����Ă���) 
			drawOpenGL();  // �`��
			last = glfwGetTime();
		}

		// �`��p�o�b�t�@�̐؂�ւ�
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	atexit(glfwTerminate);  // �v���O�����I�����̂��܂��Ȃ�

}

