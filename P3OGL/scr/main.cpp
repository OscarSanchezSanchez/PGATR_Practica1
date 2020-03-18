#include "BOX.h"
#include "auxiliar.h"


#include <gl/glew.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h> 

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <vector>
#include "Model/Model.h"

#define NUM_PARTICLES 1204*1024
#define WORK_GROUP_SIZE 128


//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

//Matrices
glm::mat4	proj = glm::mat4(1.0f);
glm::mat4	view = glm::mat4(1.0f);
glm::mat4	model = glm::mat4(1.0f);

//desplazamientos por teclado
float displacement = 0.1f;
float displacementLight = 5.0f;
//Giro de c�mara por teclado
float yaw_angle = 0.01f;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 6.0f);
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
float angulo = -3.1415 * 0.5f;

int widthVentana = 1200;
int heightVentana = 500;
float radio = 6;
glm::vec2 antPos;

//Movimiento de c�mara con el rat�n
const float orbitAngle = 0.1f;
float lastX = 0.0f;
float lastY = 0.0f;
float desplX = 0.0f;
float desplY = 0.0f;


//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////
//Texturas  
unsigned int colorTexId;  
unsigned int emiTexId;

struct program
{
	unsigned int vshader;
	unsigned int tcshader;
	unsigned int teshader;
	unsigned int gshader;
	unsigned int fshader;
	unsigned int program;

};
std::vector<program> programs(1);

struct computeProgram
{
	unsigned int shader;
	unsigned int program;
};
std::vector<computeProgram> computePrograms(1);

//Variables Uniform

int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;
int uProjectionMatrix;



//Variables uniformes posición e intesidad de la luz
int uLightPosition;
int uLightIntensity;

glm::vec3 lightPosition(0.0f);
glm::vec3 lightIntensity(0.1f);

//Texturas Uniform  
int uColorTex;  
int uEmiTex;

//Atributos VBO
int inPos;
int inColor;
int inNormal;
int inTexCoord;

//VAO
unsigned int vao;
//VBO identifiyer
unsigned int posVBO;
unsigned int colorVBO;
unsigned int normalVBO;
unsigned int texCoordVBO;
unsigned int triangleIndexVBO;

//SSBO identifyer
unsigned int posSSBO;
unsigned int velSSBO;
unsigned int colorSSBO;


//SSBO
std::vector<glm::vec4> positions;
std::vector<glm::vec4> velocities;
std::vector<glm::vec4> colors;


//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////
//!!Por implementar

//Declaración de CB
void renderFunc();
void renderParticles();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);
void mouseMotionFunc(int x, int y);

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initObj();
void initObj(Model model);
void initSSBOrender(const char* computeName, struct computeProgram* computeProgram);
void destroy();
void generateRandomPoints(std::vector<glm::vec4> &positions, std::vector<glm::vec4> &velocities, std::vector<glm::vec4> &colors);

void initShader(const char* vname, const char* fname, const char* gname, const char* tcname, const char* tename,
	struct program* program);
void initComputeShader(const char* computename, struct computeProgram* program);


//Carga el shader indicado, devuele el ID del shader
//!Por implementar
GLuint loadShader(const char *fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
//!!Por implementar
unsigned int loadTex(const char *fileName);

Model myModel("obj/teapot.obj");

int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)

	generateRandomPoints(positions,velocities,colors);

	initContext(argc, argv);
	initOGL();

	//initComputeShader();
	initSSBOrender("../shaders_P3/shader.v0.comp", &computePrograms[0]);

	
	initShader("../shaders_P3/shader.v0.vert", "../shaders_P3/shader.v0.frag", "../shaders_P3/shader.v0.geo",
		"../shaders_P3/shader.v0.tcs", "../shaders_P3/shader.v0.tes", &programs[0]);

	//initObj(myModel);

	glutMainLoop();

	destroy();

	return 0;
}
	
//////////////////////////////////////////
// Funciones auxiliares 
void initContext(int argc, char** argv){

	glutInit(&argc, argv);
	glutInitContextVersion(4, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(widthVentana, heightVentana);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Prácticas OGL");

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}
	const GLubyte* oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);
	glutMotionFunc(mouseMotionFunc);

}

void initOGL(){

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);


	proj = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 50.0f);
	view = glm::mat4(1.0f);
	view[3].z = -5.0f;

}

void destroy()
{
	glDetachShader(programs[0].program, programs[0].vshader);
	glDetachShader(programs[0].program, programs[0].tcshader);
	glDetachShader(programs[0].program, programs[0].teshader);
	glDetachShader(programs[0].program, programs[0].fshader);
	glDetachShader(programs[0].program, programs[0].gshader);
	glDeleteShader(programs[0].vshader);
	glDeleteShader(programs[0].tcshader);
	glDeleteShader(programs[0].teshader);
	glDeleteShader(programs[0].fshader);
	glDeleteShader(programs[0].gshader);
	glDeleteProgram(programs[0].program);
	glDetachShader(programs[1].program, programs[1].vshader);
	glDetachShader(programs[1].program, programs[1].tcshader);
	glDetachShader(programs[1].program, programs[1].teshader);
	glDetachShader(programs[1].program, programs[1].fshader);
	glDetachShader(programs[1].program, programs[1].gshader);
	glDeleteShader(programs[1].vshader);
	glDeleteShader(programs[1].tcshader);
	glDeleteShader(programs[1].teshader);
	glDeleteShader(programs[1].fshader);
	glDeleteShader(programs[1].gshader);
	glDeleteProgram(programs[1].program);

	if (inPos != -1) glDeleteBuffers(1, &posVBO);
	if (inColor != -1) glDeleteBuffers(1, &colorVBO);
	if (inNormal != -1) glDeleteBuffers(1, &normalVBO);
	if (inTexCoord != -1) glDeleteBuffers(1, &texCoordVBO);
	glDeleteBuffers(1, &triangleIndexVBO);
	glDeleteVertexArrays(1, &vao);

	glDeleteTextures(1, &colorTexId);  
	glDeleteTextures(1, &emiTexId);

}

void generateRandomPoints(std::vector<glm::vec4>& positions, std::vector<glm::vec4>& velocities, std::vector<glm::vec4>& colors)
{
	float auxFloat = widthVentana / 2;
	float HI = 500;
	float LO = -500;
	for (size_t i = 0; i < NUM_PARTICLES; i++)
	{

		//float r3 = LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
		glm::vec4 aux;
		aux.x = LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
		aux.y = LO + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (HI - LO)));
		aux.z = -(((float)rand() * 2) - 1) / auxFloat;
		aux.w = 1.0f;
		positions.push_back(aux);

		aux.x = 0.1f;
		aux.y = 0.1f;
		aux.z = 0.1f;
		aux.w = 1.0f;
		velocities.push_back(aux);

		aux.x = (((float)rand() * 2) - 1) / auxFloat;
		aux.y = (((float)rand() * 2) - 1) / auxFloat;
		aux.z = (((float)rand() * 2) - 1) / auxFloat;
		aux.w = 1.0f;
		colors.push_back(aux);
	}

}

void initShader(const char* vname, const char* fname, const char* gname, const char* tcname, const char* tename, struct program* program) 
{
	program->vshader = loadShader(vname, GL_VERTEX_SHADER);
	program->fshader = loadShader(fname, GL_FRAGMENT_SHADER);
	//program->gshader = loadShader(gname, GL_GEOMETRY_SHADER);
	//program->tcshader = loadShader(tcname, GL_TESS_CONTROL_SHADER);
	//program->teshader = loadShader(tename, GL_TESS_EVALUATION_SHADER);

	program->program = glCreateProgram();
	glAttachShader(program->program, program->vshader);
	glAttachShader(program->program, program->fshader);
	//glAttachShader(program->program, program->gshader);
	//glAttachShader(program->program, program->tcshader);
	//glAttachShader(program->program, program->teshader);
	glLinkProgram(program->program);

	//comprobacion de errores en el enlazado de shader al programa
	int linked;
	glGetProgramiv(program->program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(program->program, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(program->program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteProgram(program->program);
		program->program = 0;
		exit(-1);
	}

	uNormalMat = glGetUniformLocation(program->program, "normal");
	uModelViewMat = glGetUniformLocation(program->program, "modelView");
	uModelViewProjMat = glGetUniformLocation(program->program, "modelViewProj");

	uColorTex = glGetUniformLocation(program->program, "colorTex");
	uEmiTex = glGetUniformLocation(program->program, "emiTex");

	uLightPosition = glGetUniformLocation(program->program, "lightPosition");
	uLightIntensity = glGetUniformLocation(program->program, "lightIntensity");

	inPos = glGetAttribLocation(program->program, "inPos");
	inColor = glGetAttribLocation(program->program, "inColor");
	inNormal = glGetAttribLocation(program->program, "inNormal");
	inTexCoord = glGetAttribLocation(program->program, "inTexCoord");

	glUseProgram(program->program);
	if (uColorTex != -1) {
		glUniform1i(uColorTex, 0);
	}

	if (uEmiTex != -1) {
		glUniform1i(uEmiTex, 1);
	}

}

void initComputeShader(const char* computeName, struct computeProgram* computeProgram)
{
	computeProgram->shader = loadShader(computeName, GL_COMPUTE_SHADER);
	computeProgram->program = glCreateProgram();
	glAttachShader(computeProgram->program, computeProgram->shader);
	glLinkProgram(computeProgram->program);

	//comprobacion de errores en el enlazado de shader al programa
	int linked;
	glGetProgramiv(computeProgram->program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(computeProgram->program, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(computeProgram->program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteProgram(computeProgram->program);
		computeProgram->program = 0;
		exit(-1);
	}

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, posSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, velSSBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, posSSBO);
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);



}

void initObj(){

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if (inPos != -1)
	{
		glGenBuffers(1, &posSSBO);
		glBindBuffer(GL_ARRAY_BUFFER, posSSBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,
			cubeVertexPos, GL_STATIC_DRAW);
		glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);
	}
	if (inNormal != -1)
	{
		glGenBuffers(1, &normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3,cubeVertexNormal, GL_STATIC_DRAW);
		glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inNormal);
	}
	if (inTexCoord != -1)
	{
		glGenBuffers(1, &texCoordVBO);
		glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
		glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 2,cubeVertexTexCoord, GL_STATIC_DRAW);
		glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inTexCoord);
	}

	glGenBuffers(1, &triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,cubeNTriangleIndex * sizeof(unsigned int) * 3, cubeTriangleIndex,GL_STATIC_DRAW);

	colorTexId = loadTex("../img/color2.png");  
	emiTexId = loadTex("../img/emissive.png");
	model = glm::mat4(1.0f);

}

void initObj(Model model)
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if (inPos != -1)
	{
		glGenBuffers(1, &posVBO);
		glBindBuffer(GL_ARRAY_BUFFER, posVBO);
		glBufferData(GL_ARRAY_BUFFER, model.vertices.size() * sizeof(float) * 3,
			&model.vertices[0], GL_STATIC_DRAW);
		glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);
	}

	if (inNormal != -1)
	{
		glGenBuffers(1, &normalVBO);
		glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
		glBufferData(GL_ARRAY_BUFFER, model.normals.size() * sizeof(float) * 3, &model.normals[0], GL_STATIC_DRAW);
		glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inNormal);
	}

	glGenBuffers(1, &triangleIndexVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleIndexVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indices.size() * sizeof(unsigned int), &model.indices[0], GL_STATIC_DRAW);

	colorTexId = loadTex("../img/color2.png");
	emiTexId = loadTex("../img/emissive.png");
	//model = glm::mat4(1.0f);
}

void initSSBOrender(const char* computeName, struct computeProgram* computeProgram)
{
	////////////////////////////////////////////    SSBO create, bind, etc   //////////////////////////////////////////////
	

	glGenBuffers(1, &posSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec4), &positions[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, posSSBO);


	glGenBuffers(1, &velSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec4), &velocities[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, velSSBO);

	glGenBuffers(1, &colorSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec4), &colors[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, colorSSBO);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if (inPos != -1)
	{
		glBindBuffer(GL_ARRAY_BUFFER, posSSBO);
		glVertexAttribPointer(inPos, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);
	}

	if (inColor != -1)
	{
		glBindBuffer(GL_ARRAY_BUFFER, colorSSBO);
		glVertexAttribPointer(inColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inColor);
	}

	//////////////////////////////////////////////   Compile, bind and link shader   //////////////////////////////////////////////
	computeProgram->shader = loadShader(computeName, GL_COMPUTE_SHADER);
	computeProgram->program = glCreateProgram();
	glAttachShader(computeProgram->program, computeProgram->shader);
	glLinkProgram(computeProgram->program);

	//comprobacion de errores en el enlazado de shader al programa
	int linked;
	glGetProgramiv(computeProgram->program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetProgramiv(computeProgram->program, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(computeProgram->program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteProgram(computeProgram->program);
		computeProgram->program = 0;
		exit(-1);
	}
}

GLuint loadShader(const char *fileName, GLenum type){ 

	unsigned int fileLen;
	char* source = loadStringFromFile(fileName, fileLen);
	//////////////////////////////////////////////
	//Creación y compilación del Shader
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1, (const GLchar * *)& source, (const GLint*)& fileLen);
	glCompileShader(shader);
	delete[] source;

	//Comprobamos que se compiló bien
	GLint compiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled)
	{
		//Calculamos una cadena de error
		GLint logLen;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetShaderInfoLog(shader, logLen, NULL, logString);
		std::cout << fileName << std::endl;
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteShader(shader);
		exit(-1);
	}

	return shader; 
}

unsigned int loadTex(const char *fileName){ 
	unsigned char* map;  
	unsigned int w, h;  
	map = loadTexture(fileName, w, h);

	if (!map) { 
		std::cout << "Error cargando el fichero: " 
			<< fileName << std::endl;  
		exit(-1); 
	}

	unsigned int texId;  
	glGenTextures(1, &texId);  
	glBindTexture(GL_TEXTURE_2D, texId); 
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, 
		GL_UNSIGNED_BYTE, (GLvoid*)map);

	delete[] map;

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
		GL_LINEAR_MIPMAP_LINEAR);  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);  
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);

	return texId;

}

void renderFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	glUseProgram(computePrograms[0].program);
	glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glUseProgram(programs[0].program);
	renderParticles();


	/*
	//Texturas  
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, colorTexId);

	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, emiTexId);
	*/
	glutSwapBuffers();
}

void renderParticles()
{
	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * modelView;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	if (uProjectionMatrix != -1)
	{
		glUniformMatrix4fv(uProjectionMatrix, 1, GL_FALSE, &proj[0][0]);
	}

	if (uModelViewMat != -1)
	{
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE, &modelView[0][0]);
	}

	if (uModelViewProjMat != -1)
	{
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE, &modelViewProj[0][0]);
	}

	if (uNormalMat != -1)
	{
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE, &normal[0][0]);
	}

	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
	glBindVertexArray(0);
}

void resizeFunc(int width, int height)
{
	float aspectRatio = (float)width / (float)height;

	proj = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 50.0f);

	glViewport(0, 0, width, height);

	glutPostRedisplay();

}

void idleFunc()
{
	model = glm::mat4(1.0f);
	static float angle = 0.0f;
	angle = (angle > 3.141592f * 2.0f) ? 0 : angle + 0.01f;
	//models[0] = glm::rotate(models[0], angle, glm::vec3(1.0f, 1.0f, 0.0f));
	glutPostRedisplay();

	//models[1] = glm::mat4(1.0f);
	//rotacion sobre eje y
	//models[1] = glm::rotate(models[1], angle, glm::vec3(0, 4, 0));

	//translacion sobre x para desplazar el cuadrado
	//models[1] = glm::translate(models[1], glm::vec3(3, 0, 0));

	//rotacion sobre Y para simular la orbitacion del objeto
	//models[1] = glm::rotate(models[1], angle, glm::vec3(0, 1, 0));

	glutPostRedisplay();
}

void keyboardFunc(unsigned char key, int x, int y)
{
	// Movimiento FPS
	float speed = 0.1f;
	if (key == 'w' || key == 'W')
		cameraPos += speed * cameraFront;
	else if (key == 's' || key == 'S')
		cameraPos -= speed * cameraFront;
	else if (key == 'a' || key == 'A')
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
	else if (key == 'd' || key == 'D')
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * speed;
	else if (key == 'x' || key == 'X') {
		angulo = (angulo < 6.2830f) ? angulo + 0.1f : 0.0f;
		cameraFront.x = cos(angulo);
		cameraFront.z = sin(angulo);
	}
	else if (key == 'z' || key == 'Z') {
		angulo = (angulo > -6.2830f) ? angulo - 0.1f : 0.0f;
		cameraFront.x = cos(angulo);
		cameraFront.z = sin(angulo);
	}
	cameraFront = glm::normalize(cameraFront);
	view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    glutPostRedisplay();

}

void mouseFunc(int button, int state, int x, int y)
{
    if (state == 0)
        std::cout << "Se ha pulsado el boton ";
    else
        std::cout << "Se ha soltado el boton ";

    if (button == 0) std::cout << "de la izquierda del raton " << std::endl;
    if (button == 1) std::cout << "central del raton " << std::endl;
    if (button == 2) std::cout << "de la derecha del raton " << std::endl;

    std::cout << "en la posicion " << x << " " << y << std::endl << std::endl;
}

void mouseMotionFunc(int x, int y)
{

    float xOffset = (float)x - lastX;
    float yOffset = (float)y - lastY;

    lastX = (float)x;
    lastY = (float)y;

    desplX += xOffset;
    desplY += yOffset;

    glm::mat4 rotation(1.0f);

    view = glm::rotate(view, orbitAngle, glm::vec3(desplY, desplX, 0.0));

    glutPostRedisplay();
}

