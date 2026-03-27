// Exodus.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include<math.h>
#define GLEW_STATIC
#include<glew.h>
#include<GL/glut.h>
#include<GL/GL.h>
#include<vector>
#include<Windows.h>
#include<mmsystem.h>
#define rads 3.14159/180 
#define STB_IMAGE_IMPLEMENTATION
#include<stb_image.h> 
#define TINYOBJLOADER_IMPLEMENTATION 
#include<tiny_obj_loader.h> 
#define GRIDSIZE 0.7
float prx, pry, px, py, pz; 
bool onobject = true;  
float speed = 0.3;
GLuint texs[37]; 
float vx, vy, vz;  
float oldpx, oldpy, oldpz;
bool cubecollide(float px, float py, float pz, float x1, float x2, float y1, float y2, float z1, float z2)
{
	return(px > x1 && px<x2 && py>y1 && py<y2 && pz>z1 && pz < z2);
}

GLuint texload(const char* file)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	int texw, texh, channels;
	unsigned char* filedata = stbi_load(file, &texw, &texh, &channels, 0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	if (!filedata)
	{
		exit(1);
	}
	GLenum format = (channels == 4) ? GL_RGBA : (channels == 3) ? GL_RGB : (channels == 1) ? GL_RED : GL_RGB;
	glTexImage2D(GL_TEXTURE_2D, 0, format, texw, texh, 0, format, GL_UNSIGNED_BYTE, filedata);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(filedata);
	return textureID;
}

struct tile
{
	float fx, fy, fz;
	float scalex, scaley, scalez;
	GLuint tex;
	void Floor()
	{
		glPushMatrix();
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex);


		glTranslatef(fx, fy, fz);
		glScalef(scalex, scaley, scalez);
		glColor3f(1, 1, 1);
		glBegin(GL_QUADS);
		glNormal3f(0, 1, 0);
		glTexCoord2f(0, 0);
		glVertex3f(-1.0, 0, -1.0);
		glTexCoord2f(0, 1);
		glVertex3f(1.0, 0, -1.0);
		glTexCoord2f(1, 1);
		glVertex3f(1.0, 0, 1.0);

		glTexCoord2f(1, 0);
		glVertex3f(-1.0, 0, 1.0);
		glEnd();
		if (cubecollide(px, py, pz, fx - (scalex), fx + (scalex), fy - 0.1, fy + 3, fz - (scalez), fz + (scalez)))
		{
			std::cout << "floor is here\n";
			onobject = true;
			if (tex == texs[5])
			{
				vy += 2;
			}
		}

		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
	}

};
std::vector<tile>tilez;

struct playermove
{
	bool up, down, left, right, upward, downward, lookleft, lookright, lookup, lookdown;
};
playermove move = { false, false, false, false,false,false,false,false,false,false };

struct model
{
	std::vector<tinyobj::real_t>vertix;
	std::vector<tinyobj::real_t>normals;
	GLuint vbos[2];
};
model coolBoy, hammer;
void OBJloader(model& M, std::string path)
{
	tinyobj::attrib_t atrib;
	std::vector<tinyobj::shape_t>shapes;
	std::vector<tinyobj::material_t>mats;

	bool retun = tinyobj::LoadObj(&atrib, &shapes, &mats, NULL, NULL, path.c_str());
	M.vertix.clear();
	M.normals.clear();
	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{
			int vIdx = 3 * index.vertex_index;
			M.vertix.push_back(atrib.vertices[vIdx + 0]);
			M.vertix.push_back(atrib.vertices[vIdx + 1]);
			M.vertix.push_back(atrib.vertices[vIdx + 2]);
			if (index.normal_index >= 0)
			{
				int nIdx = 3 * index.normal_index;
				M.normals.push_back(atrib.normals[nIdx + 0]);
				M.normals.push_back(atrib.normals[nIdx + 1]);
				M.normals.push_back(atrib.normals[nIdx + 2]);
			}
			else
			{
				M.normals.push_back(0);
				M.normals.push_back(1);
				M.normals.push_back(0);
			}

		}
	}
	glGenBuffers(2, M.vbos);

	// Vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, M.vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, M.vertix.size() * sizeof(float), M.vertix.data(), GL_STATIC_DRAW);

	// Normal buffer
	glBindBuffer(GL_ARRAY_BUFFER, M.vbos[1]);
	glBufferData(GL_ARRAY_BUFFER, M.normals.size() * sizeof(float), M.normals.data(), GL_STATIC_DRAW);
}
void OBJdrawer(model M, float x, float y, float z, float angle, float scale)
{
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glTranslatef(x, y, z);
	glRotatef(angle, 0, 1, 0);
	glScalef(scale, scale, scale);
	glColor4f(1, 1, 1, 1);
	glBindTexture(GL_TEXTURE_2D, texs[14]);
	glBindBuffer(GL_ARRAY_BUFFER, M.vbos[0]);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, M.vbos[1]);
	glNormalPointer(GL_FLOAT, 0, 0);

	glDrawArrays(GL_TRIANGLES, 0, M.vertix.size() / 3);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void Sprite(float sx, float sy, float sz, float scale, GLuint SpriteTex)
{
	glPushMatrix();

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D, SpriteTex);

	glTranslatef(sx, sy, sz);
	glRotatef(pry, 0, 1, 0);
	glScalef(scale, scale, scale);
	glColor4f(1, 1, 1, 1);

	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex3f(1, 1, 0);
	glTexCoord2f(1, 0); glVertex3f(-1, 1, 0);
	glTexCoord2f(1, 1); glVertex3f(-1, -1, 0);
	glTexCoord2f(0, 1); glVertex3f(1, -1, 0);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glPopMatrix();
}

void Player()
{
	oldpx = px;
	oldpz = pz;
	if (move.up == true)
	{
		px += cos((pry + 90) * rads) * speed;
		pz -= sin((pry + 90) * rads) * speed;
	}
	if (move.down == true)
	{
		px += cos((pry - 90) * rads) * speed;
		pz -= sin((pry - 90) * rads) * speed;
	}
	if (move.left == true)
	{
		px += cos((pry + 180) * rads) * speed;
		pz -= sin((pry + 180) * rads) * speed;
	}
	if (move.right == true)
	{
		px += cos((pry)*rads) * speed;
		pz -= sin((pry)*rads) * speed;
	}
	if (move.lookleft == true)
	{
		pry += 1;
	}
	if (move.lookright == true)
	{
		pry -= 1;
	}
	if (move.lookup == true)
	{
		prx += 1;
	}
	if (move.lookdown == true)
	{
		prx -= 1;
	}
	if (move.upward == true)
	{
		py += speed;
	}
	if (py < -12)
	{
		px = 0;
		py = 3;
		pz = 0;
	}

	glRotatef(-prx, 1, 0, 0);
	glRotatef(-pry, 0, 1, 0);
	glTranslatef(-px, -py, -pz);
}

void keys1(unsigned char key, int, int)
{
	if (key == 'w')
	{
		move.up = true;
	}
	if (key == 's')
	{
		move.down = true;
	}
	if (key == 'a')
	{
		move.left = true;
	}
	if (key == 'd')
	{
		move.right = true;
	}
	if (key == 32)
		move.upward = true;

	glutPostRedisplay();
}
void special(int key, int, int)
{
	if (key == GLUT_KEY_UP)
	{
		move.lookup = true;
	}
	if (key == GLUT_KEY_DOWN)
	{
		move.lookdown = true;
	}
	if (key == GLUT_KEY_LEFT)
	{
		move.lookleft = true;
	}
	if (key == GLUT_KEY_RIGHT)
	{
		move.lookright = true;
	}
}
void notsospecial(int key, int, int)
{
	if (key == GLUT_KEY_UP)
	{
		move.lookup = false;
	}
	if (key == GLUT_KEY_DOWN)
	{
		move.lookdown = false;
	}
	if (key == GLUT_KEY_LEFT)
	{
		move.lookleft = false;
	}
	if (key == GLUT_KEY_RIGHT)
	{
		move.lookright = false;
	}
}
void keys2(unsigned char key, int, int)
{
	if (key == 'w')
	{
		move.up = false;
	}
	if (key == 's')
	{
		move.down = false;
	}
	if (key == 'a')
	{
		move.left = false;
	}
	if (key == 'd')
	{
		move.right = false;
	}
	if (key == 'e')
	{
		move.upward = false;
	}
	if (key == 'q')
	{
		move.downward = false;
	}
	if (key == 32)
		move.upward = false;
}
struct block
{
	int x, y, z;
	const char* direction; 
	int tex; 
	float angle = 0;
	void blockDrawer()
	{
		glPushMatrix();
		if (direction == "forward")
			angle = 0;
		if (direction == "backward")
			angle = 180;
		if (direction == "left")
			angle = -90;
		if (direction == "right")
			angle = 90;

		glTranslatef(x, y, z); 
		glRotatef(angle,0,1,0);	
		glutSolidCube(1);
		glPopMatrix();
	}
};
std::vector<block>blockz;
void blockPlacer(float x,float y,float z,int radius,int numSegs)
{
	for (int i = 0; i < 300; i++)
	{
		float theta = radius * 3.14159 * i/ numSegs; 
		int bx = radius*cos(theta); 
		int bz = radius*sin(theta);
		blockz.push_back({bx,0,bz});
	}
}
void display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	Player();  
	static float cChange = 0;
	cChange += 3;
	glColor3f(sin(cChange*rads),cos(cChange*rads), cos(cChange*rads)*sin(cChange*rads)); 
	glutSolidTeapot(1);
	for (auto& b:blockz)
	{
		b.blockDrawer();
	}
	glutSwapBuffers();
}
void INIT()
{
	glEnable(GL_DEPTH_TEST);
	GLfloat l0_diffuse[] = { 1,1,1,0 };
	GLfloat specular[] = { 0.1,0.1,0.1 };
	GLfloat shiny[] = { 128 };
	GLfloat ambeint[] = { 0.1,0.1,0.1 };
	glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, l0_diffuse);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambeint);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shiny);
	glEnable(GL_NORMALIZE);
}

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	gluPerspective(70, w/h, 0.1, 10000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void clock(int)
{
	glutPostRedisplay();
	float pdx = px + cos((pry + 90 )* rads);
	float pdz = pz - sin((pry + 90) * rads);
	GLfloat pos[] = {px,py+25,pz};
	glLightfv(GL_LIGHT0, GL_POSITION, pos); 
	static float timer = 0;
	timer += 0.1;  
	static int r = 7;
	if (timer > 2)
	{
		blockPlacer(73, 0, 3, r, 100);
		r = r + 1;
		timer =0;
	}
	glutTimerFunc(10,clock,0); 
}

int main()
{
	glewInit();

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH|GLUT_RGB); 
	glutInitWindowSize(400,400);
	glutInitWindowPosition(500, 100);
	glutCreateWindow("Praise GOD for he is merciful"); 
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutSpecialUpFunc(notsospecial);
	glutKeyboardFunc(keys1);
	glutKeyboardUpFunc(keys2);
	glutTimerFunc(0, clock, 0);
	INIT();
	glutMainLoop(); 
	return(0);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
