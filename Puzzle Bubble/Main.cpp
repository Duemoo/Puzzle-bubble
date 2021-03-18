/*//////////////////////////////////////////////////////////////////////
*우클릭 -> speed 선택을 통해 버블이 발사되는 속도 조절이 가능합니다.   *
*이 때, 발사 시 버블의 속도 이외의 게임 요소에는 영향이 없습니다.	   *
*우클릭 메뉴 또는 r/q 키로 프로그램 리셋/종료가 가능합니다.		       *
*레벨 및 난이도는 별도의 스테이지가 아니라, 스코어가 일정 이상 올라가면*
*버블의 색깔 수가 점점 추가되고, timebar의 시간제한이 줄어드는 방식으로*
*구현했습니다.														   *
*배경음악의 출처는 https://www.youtube.com/watch?v=NGazythwAkU 입니다. *
*배경화면은 지인이 직접 그려준 그림으로, 온라인 출처가 없습니다.	   *
//////////////////////////////////////////////////////////////////////*/

#include <iostream>
#include <vector>
#include <GL/glut.h>
#include <windows.h>
#pragma comment(lib, "winmm")
#include <mmsystem.h>
#include <chrono>
#include <random>
#include "Bubble.h"
#include "Light.h"
#include "Arrow.h"
#include "Timebar.h"
#include <FI/FreeImage.h>
#include <math.h>
#include <string>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

using namespace std;

#define WIDTH 340
#define HEIGHT 560
#define FIELD_TOP 180
#define DEADLINE -220
#define LOADHEIGHT -250
#define NEXTHEIGHT -250

#define boundaryX WIDTH/2
#define boundaryY HEIGHT/2

#define fps 1000.f/200.f


enum MODE { CTRL, LAUNCH, EXPLODE, DROP, OVER};
int Mode;
unsigned int score = 0;
int speed = 2;
int line = 0;
float angle = 0;
bool isGameOver = false;
int progression = 0;

vector<Bubble> bubbles;
vector<Bubble> droppingBubbles;
Light light(boundaryX, boundaryY, boundaryX / 2, GL_LIGHT0);
Arrow arrow;
Timebar timebar;

auto start = chrono::system_clock::now();
auto now = chrono::system_clock::now();
auto frame = chrono::system_clock::now();
std::chrono::duration<double> sec;

vector<Material> genColors() {
	Material mtl1;
	mtl1.setEmission(0.1, 0.1, 0.1, 1);
	mtl1.setAmbient(0.3, 0.6, 0.4, 1);
	mtl1.setDiffuse(0.5, 0.5, 0.5, 1);
	mtl1.setSpecular(1.0, 1.0, 1.0, 1);
	mtl1.setShininess(10);

	Material mtl2(mtl1), mtl3(mtl1), mtl4(mtl1), mtl5(mtl1), mtl6(mtl1), mtl7(mtl1), white(mtl1);
	mtl2.setAmbient(1, 0.1, 0.1, 1);
	mtl3.setAmbient(1, 0, 1, 1);
	mtl4.setAmbient(0.1, 0, 1, 1);
	mtl5.setAmbient(0.3, 0.5, 0.7, 1);
	mtl6.setAmbient(1, 1, 0, 1);
	mtl7.setAmbient(0.2, 0.2, 0.3, 1);
	white.setAmbient(1, 1, 1, 1);
	
	vector<Material> c;
	c.push_back(mtl1);
	c.push_back(mtl2);
	c.push_back(mtl3);
	c.push_back(mtl4);
	c.push_back(mtl5);
	c.push_back(mtl6);
	c.push_back(mtl7);
	c.push_back(white);

	for (int i = 0; i < c.size(); i++) {
		c[i].setID(i + 1);
	}

	return c;
}
vector<Material> colors = genColors();

static GLuint textureID;
GLubyte* textureData;
int imageWidth, imageHeight;

//Image file processing/////////////////////////////////////////////////
FIBITMAP* createBitMap(char const* filename) {
	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(filename, 0);
	if (format == -1) {
		std::cout << "Could not find image: " << filename << " - Aborting." << endl;
		exit(-1);
	}
	if (format == FIF_UNKNOWN) {
		std::cout << "Couldn't determine file format - attempting to get from file extension..." << endl;
		format = FreeImage_GetFIFFromFilename(filename);
		if (!FreeImage_FIFSupportsReading(format)) {
			std::cout << "Detected image format cannot be read!" << endl;
			exit(-1);
		}
	}
	FIBITMAP* bitmap = FreeImage_Load(format, filename);
	int bitsPerPixel = FreeImage_GetBPP(bitmap);
	FIBITMAP* bitmap32;
	if (bitsPerPixel == 32) {
		std::cout << "Source image has " << bitsPerPixel << " bits per pixel. Skipping conversion." << endl;
		bitmap32 = bitmap;
	}
	else {
		std::cout << "Source image has " << bitsPerPixel << " bits per pixel. Converting to 32-bit colour." << endl;
		bitmap32 = FreeImage_ConvertTo32Bits(bitmap);
	}
	return bitmap32;
}
void generateTexture() {
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight,
		0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, textureData);
}
void initTexture() {
	FIBITMAP* bitmap32 = createBitMap("Background.jpg");
	imageWidth = FreeImage_GetWidth(bitmap32);
	imageHeight = FreeImage_GetHeight(bitmap32);
	textureData = FreeImage_GetBits(bitmap32);
	generateTexture();
}
void drawSquareWithTexture() {
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(-170, -280, 0.0);
	glTexCoord2f(0, 1); glVertex3f(-170, 280, 0.0);
	glTexCoord2f(1, 1); glVertex3f(170, 280, 0.0);
	glTexCoord2f(1, 0); glVertex3f(170, -280, 0.0);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}
////////////////////////////////////////////////////////////////////////


//Basic Functions///////////////////////////////////////////////////////
int genRand(int min, int max) {
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> dis(min, max);
	return dis(gen);
}
void init() {
	PlaySound(TEXT("Soul&Mind - E's Jammy Jam.wav"), NULL, SND_ASYNC | SND_FILENAME | SND_LOOP);
	initTexture();
	Mode = CTRL;
	std::cout << "current mode:" << Mode << endl;

	light.setAmbient(0.5, 0.5, 0.5, 1.0);
	light.setDiffuse(0.7, 0.7, 0.7, 1.0);
	light.setSpecular(1.0, 1.0, 1.0, 1.0);

	Bubble bubble1(20, 20);

	bubble1.setMTL(colors[genRand(0, 4)]);
	bubble1.setColorID(bubble1.getMtl().getID());
	Vector3f now(0, LOADHEIGHT, 0);
	bubble1.setCenter(now);
	bubbles.push_back(bubble1);

	Bubble bubble2(20, 20);
	bubble2.setMTL(colors[genRand(0, 4)]);
	bubble2.setColorID(bubble2.getMtl().getID());
	Vector3f next(-100, NEXTHEIGHT, 0);
	bubble2.setCenter(next);
	bubbles.push_back(bubble2);

	start = std::chrono::system_clock::now();
}
void load() {
	bubbles[bubbles.size() - 1].setCenter(Vector3f(0, LOADHEIGHT, 0));
	Bubble b(20,20);
	if (score<300){ b.setMTL(colors[genRand(0, 4)]); }
	if (score >=300 && score<5000) { b.setMTL(colors[genRand(0, 5)]); }
	if (score >= 600) { b.setMTL(colors[genRand(0, 6)]); }
	
	b.setColorID(b.getMtl().getID());
	b.setCenter(Vector3f(-100, NEXTHEIGHT, 0));
	bubbles.push_back(b);
	progression++;
}
float distance(Vector3f v1, Vector3f v2) {
	float a = sqrt((v1.getX() - v2.getX()) * (v1.getX() - v2.getX()) + (v1.getY() - v2.getY())* (v1.getY() - v2.getY()));
	return a;
}
float time() {
	return 0;
}
void draw_characters(void* font, const char* c, float x, float y) {
	glColor3f(1, 1, 1);
	glRasterPos2f(x, y);
	for (int i = 0; i < strlen(c); i++)
		glutBitmapCharacter(font, c[i]);
}
void reset() {
	bubbles.clear();
	droppingBubbles.clear();
	line = 0;
	score = 0;
	angle = 0;
	progression = 0;
	timebar.setTime(10);
	isGameOver = false;
	init();
}
void sub_menu_function(int option) {
	printf("Sub menu %d has been selected\n", option);
	if (option == 1) { speed = 1;}
	if (option == 2) { speed = 2;}
	if (option == 3) { speed = 3;}
}
void main_menu_function(int option) {
	printf("Main menu %d ahs been selected\n", option);
	if (option == 998)
		reset();
	if (option == 999)
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		exit(0);
}
string scoring(unsigned int s) {
	if (s >= 9999) return "9999";
	else {
		string str = to_string(s);
		return str;
	}
	return "3";
}
void drawScore() {
	string strScore = scoring(score);
	const char* c = strScore.c_str();
	if (0 <= score && score < 10)
		draw_characters(GLUT_BITMAP_HELVETICA_18, c, -85.f, 220.f);
	else if (10 <= score && score <100)
		draw_characters(GLUT_BITMAP_HELVETICA_18, c, -90.f, 220.f);
	else if (100 <= score && score < 1000)
		draw_characters(GLUT_BITMAP_HELVETICA_18, c, -95.f, 220.f);
	else if (100 <= score && score < 1000)
		draw_characters(GLUT_BITMAP_HELVETICA_18, c, -97.5, 220.f);
	else if (1000 <= score)
		draw_characters(GLUT_BITMAP_HELVETICA_18, c, -100.f, 220.f);
}
////////////////////////////////////////////////////////////////////////

bool isNearBorder(Bubble a) {
	float height = FIELD_TOP;
	float gap = 20 * sqrt(3);
	for (int i = 0; i < 11;i++) {
		height -= gap;
		if (height-6 < a.getCenter().getY() < height+6) return 1;
	}
	return 0;
}
vector<float> nearestBorder(Bubble a) {
	float line = 0;
	float height = float(FIELD_TOP);
	vector<float> result;
	float gap = 20 * sqrt(3);
	for (int i = 0; i < 15; i++) {
		height -= gap;
		if ((height - gap/2 < a.getCenter().getY()) && (a.getCenter().getY() < height + gap/2)) {
//			cout << "current height: " << a.getCenter().getY() << endl;
	//		cout << "nearest border: " << height << endl << i << endl << endl;
			result.push_back(i);
			result.push_back(height);
			std::cout << "Nearestborder result: ";
			for (int i = 0; i < result.size(); i++) {
				std::cout << result[i] << ", ";
			}
			std::cout << endl;
			if (i == 10) isGameOver = true;
			Mode = OVER;
			return result; 
		}
	}
}

void gameOver() {
	for (auto &i : bubbles) {
		i.setMTL(colors[7]);
	}
}

bool contact(Bubble a, int num) {
	if (a.getCenter().getY() >= FIELD_TOP) {return 1;}
	else if (isNearBorder(a)) {
		for (int i = 0; i < bubbles.size()-2;i++) {
			if(1 < distance(a.getCenter(), bubbles[i].getCenter()) && distance(a.getCenter(), bubbles[i].getCenter())<num){
			//	cout << "contact!" << endl;
			return 1;
			}
		}
	}
	return 0;
}
void magnet(Bubble& a) {
	if (a.getCenter().getY() >= FIELD_TOP-10*sqrt(3)) {
		a.setCenter(Vector3f(roundf((a.getCenter()[0] + 20) / 40) * 40 - 20, FIELD_TOP, 0));
		Mode = EXPLODE;
	}
	else{
		for (int i = 0; i < bubbles.size() - 2; i++) {
			if (distance(a.getCenter(), bubbles[i].getCenter()) <= 40) {
				Mode = EXPLODE;
				if ((int)nearestBorder(a)[0] % 2 == 1) a.setCenter(Vector3f(roundf((a.getCenter()[0] + 20) / 40) * 40 - 20, nearestBorder(a)[1], 0));
				else if ((int)nearestBorder(a)[0] % 1 == 0) a.setCenter(Vector3f(roundf((a.getCenter()[0] + 0.1) / 40) * 40, nearestBorder(a)[1], 0));
				//cout << "current pos: " << a.getCenter().getX() << ",  " << a.getCenter().getY() << endl;
			}
		}
	}
}

void markExplosion(vector<Bubble>& b, int num) {
	int tmp1 = b[num].getColorID();
	int tmp2;
	b[num].setChecked(true);
	for (int i = 0; i < bubbles.size() - 3; i++) {
		if (2 < distance(b[num].getCenter(), bubbles[i].getCenter()) && distance(b[num].getCenter(), bubbles[i].getCenter())<41){
			if (bubbles[i].getChecked() == false) {
				tmp2 = bubbles[i].getColorID();
				if (tmp1 == tmp2) {				 
					bubbles[i].setMarked(true);
					//delete current;
					markExplosion(b, i);
				}
			}
		}	
	}
}
vector<Bubble> updateExplosion(vector<Bubble>& b){
	int count = 0;
	for (auto i : b) {
		{if (i.getMarked() == true) count += 1;
		}
	}

	//cout << "current count: " << count << endl;

	if (count >= 3) {
		score += 10 * count;
		vector<Bubble> newbubbles;
		for (auto &i : b) {
			if (i.getMarked() == false) {
				newbubbles.push_back(i);
			}
		}
		for (auto &i : newbubbles) {
			i.setMarked(false);
		}
		std::cout << "explosion updated. current bubbles: " << newbubbles.size() << endl;
		return newbubbles;
	}
	else {
		for (auto &i : b) {
			i.setMarked(false);
		}
		std::cout << "explosion updated. current bubbles: " << b.size() << endl;
		return b;
	}
}

void markDrop(vector<Bubble>& b, int num){
	if (b[num].getChecked() == false) {
		b[num].setChecked(true);
	}
		if (b[num].getCenter().getY() == FIELD_TOP) { b[num].setMarked(true); }
		for (int i = 0; i < bubbles.size() - 2; i++) {
			if (2 < distance(b[num].getCenter(), bubbles[i].getCenter()) && distance(b[num].getCenter(), bubbles[i].getCenter()) < 41) {
				if (bubbles[i].getChecked() == false) {
					if (b[num].getMarked() == true) {
						bubbles[i].setMarked(true);
					}
					else if (bubbles[i].getMarked() == true) {
						b[num].setMarked(true);
					}
					bubbles[i].setChecked(true);
					markDrop(b, i);
						
				}
			}
		}
}
vector<Bubble> updateDrop(vector<Bubble>& b) {
	vector<Bubble> newbubbles;
	b[b.size() - 1].setMarked(true);
	b[b.size() - 2].setMarked(true);
	for (int i = 0; i < b.size();i++) {
		if (b[i].getMarked() == true) {
			newbubbles.push_back(b[i]);
		}
		else {
			b[i].setVelocity(Vector3f(0, -11, 0));
			droppingBubbles.push_back(b[i]);
		}
	}
	//cout << "Drop updated: returned bubbles: " << newbubbles.size() << endl;
	score += 30 * droppingBubbles.size();
	return newbubbles;
}

void idle() {
	now = chrono::system_clock::now();
	auto mill = chrono::duration_cast<std::chrono::milliseconds>(now - frame);
	if (mill.count()>=fps){
		frame = now;

	if (isGameOver == false) {
		if (Mode == CTRL) {
			sec = chrono::system_clock::now() - start;
			timebar.setLength(63 * (1 - sec.count() / timebar.getTime()));

			if (timebar.getLength() < 0.1) {
				bubbles[bubbles.size() - 2].launch(arrow.getAngle(), speed);
				Mode = LAUNCH;
			}
		}
		if (Mode == LAUNCH) {
			if (contact(bubbles[bubbles.size() - 2], 40) == false) {
				bubbles[bubbles.size() - 2].move();
			}
		}
		else if (Mode == EXPLODE) {
			bubbles[bubbles.size() - 3].setMarked(true);
			markExplosion(bubbles, bubbles.size() - 3);
			bubbles = updateExplosion(bubbles);
			for (auto& i : bubbles) {
				i.setChecked(false);
			}
			std::cout << "Mode changed to DROP, current bubbles: " << bubbles.size() << endl;
			for (int i = 0; i < bubbles.size() - 2; i++) {
				markDrop(bubbles, i);
			}
			bubbles = updateDrop(bubbles);
			for (auto& i : bubbles) {
				i.setChecked(false);
				i.setMarked(false);
			}
			Mode = DROP;
		}
		else if (Mode == DROP) {
			if (droppingBubbles.size() > 0) {
				for (int i = 0; i < droppingBubbles.size(); i++) {
					if (droppingBubbles[i].getCenter().getY() > -HEIGHT) {
						droppingBubbles[i].move();
					}
					else droppingBubbles[i].setMarked(true);
				}
				vector<Bubble> tmp;
				for (auto& i : droppingBubbles) {
					if (i.getMarked() == false)
						tmp.push_back(i);
				}
				droppingBubbles = tmp;
				tmp.clear();
			}
			else if (droppingBubbles.size() == 0) {
				start = std::chrono::system_clock::now();
				timebar.setTime(10 - 0.2 * progression);
				Mode = CTRL;
			}
		}
	}
	if (isGameOver == true) {
		gameOver();
	}
	}
}

void renderScene() {

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-boundaryX, boundaryX, -boundaryY, boundaryY, -100.0, 100.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Draw 2D
	drawSquareWithTexture();
	draw_characters(GLUT_BITMAP_HELVETICA_18, "NEXT", -125.f, -223.f);
	draw_characters(GLUT_BITMAP_HELVETICA_18, "SCORE", -115.f, 247.f);
	draw_characters(GLUT_BITMAP_HELVETICA_18, "TIME", 65.f, 247.f);
	drawScore();
	timebar.draw();

	// Draw 3D
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(light.getID());
	light.draw();

	idle();

	if (contact(bubbles[bubbles.size() - 2],40)==true) {
		magnet(bubbles[bubbles.size()-2]);
		std::cout << bubbles[bubbles.size()-2].getCenter().getX() << endl;
		load();
		Mode = EXPLODE;
	}

	for (int i = 0; i < bubbles.size(); i++) {
		bubbles[i].draw();
	}
	for (int i = 0; i < droppingBubbles.size(); i++) {
		droppingBubbles[i].draw();
	}

	arrow.draw();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(light.getID());

	glutSwapBuffers();
}


///////////////////////////////////////////////////////////////////////
void processNormalKeys(unsigned char key, int x, int y) {
//	cout << "normalkeys called" << endl;
	if (key == 'q') exit(0);
	else if (key == 'r') reset();
	if (Mode == CTRL) {
		switch (key) {
		case 'p':
			break;
		case ' ':
			std::cout << "launch!" << endl;
			bubbles[bubbles.size() - 2].launch(arrow.getAngle(), speed);
			Mode = LAUNCH;
			break;
		default:
			break;
		}
	}
}
void processSpecialKeys(int key, int x, int y) {
	if (Mode == CTRL) {
		//cout << "specialkeys called" << endl;
		switch (key) {
		case GLUT_KEY_RIGHT:
			arrow.rotate(1);
			renderScene();
			break;
		case GLUT_KEY_LEFT:
			arrow.rotate(-1);
			renderScene();
			break;
		default:
			break;
		}
	}
}

void main(int argc, char** argv) {

	// check memory leaks


	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(650, 300);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Puzzle Bubble!");
	init();

	// popup menu
	int mainmenu, submenu;

	submenu = glutCreateMenu(sub_menu_function);
	glutAddMenuEntry("Slow", 1);
	glutAddMenuEntry("Medium", 2);
	glutAddMenuEntry("Fast", 3);

	mainmenu = glutCreateMenu(main_menu_function);
	glutAddSubMenu("Speed", submenu);
	glutAddMenuEntry("Reset", 998);
	glutAddMenuEntry("Quit", 999);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// register callbacks
	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutKeyboardFunc(processNormalKeys);
	glutSpecialFunc(processSpecialKeys);

	// enter GLUT event processing cycle
	glutMainLoop();
}