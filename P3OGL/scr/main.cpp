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
#include "Constants.h"


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
float time = 0.0f;

float cameraDistanceZ = 3.0f;
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, cameraDistanceZ);
glm::vec3 cameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
float angulo = -3.1415 * 0.5f;

int widthVentana = 500;
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
unsigned int alphaTexId;

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
std::vector<computeProgram> computePrograms(2);

//Variables Uniform

int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;
int uProjectionMatrix;
int uStage;
int uSubStage;
int uAtractor;
int uIncrementx;
int uIncrementz;

glm::vec3 atractor(0.0f);

//Variables uniformes posición e intesidad de la luz
int uLightPosition;
int uLightIntensity;

glm::vec3 lightPosition(0.0f);
glm::vec3 lightIntensity(0.1f);

//Texturas Uniform  
int uColorTex;  
int uEmiTex;
int uAlphaTex;

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
unsigned int oldPosSSBO;
unsigned int colorSSBO;
unsigned int spawnSSBO;
unsigned int spawnVelSSBO;
unsigned int spawnOldSSBO;


//SSBO
std::vector<glm::vec4> positionsParticles;
std::vector<glm::vec4> velocitiesParticles;
std::vector<glm::vec4> oldpositionsParticles;
std::vector<glm::vec4> colorParticles;
std::vector<glm::vec4> spawn;
std::vector<glm::vec4> spawnVel;
std::vector<glm::vec4> spawnOld;


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
void initSortCompute(const char* computeName, struct computeProgram* computeProgram);
void destroy();
void generateRandomPoints();
void generateRandomPoints2();

void initShader(const char* vname, const char* fname, const char* gname, const char* tcname, const char* tename,
	struct program* program);
void initComputeShader(const char* computename, struct computeProgram* program);

void firstStepVerlet();

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
	initContext(argc, argv);
	initOGL();

	generateRandomPoints2();
	firstStepVerlet();

	initShader("../shaders_P3/shader.v0.vert", "../shaders_P3/shader.v0.frag", "../shaders_P3/shader.v0.geo",
		"../shaders_P3/shader.v0.tcs", "../shaders_P3/shader.v0.tes", &programs[0]);

	initSSBOrender("../shaders_P3/shader.verlet2.comp", &computePrograms[0]);
	initSortCompute("../shaders_P3/shader.bitonicSort.comp", &computePrograms[1]);

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
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_CONSTANT_ALPHA);
	//glBlendFunc(GL_SRC_COLOR, GL_SRC_ALPHA);
	glBlendFunc(GL_SRC_COLOR, GL_DST_ALPHA);
	glBlendEquation(GL_FUNC_ADD);


	proj = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 50.0f);
	view = glm::mat4(1.0f);
	view[3].z = -cameraDistanceZ;

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

	glDeleteShader(computePrograms[0].shader);
	glDeleteProgram(computePrograms[0].program);
	glDeleteShader(computePrograms[1].shader);
	glDeleteProgram(computePrograms[1].program);


	if (inPos != -1) glDeleteBuffers(1, &posVBO);
	if (inColor != -1) glDeleteBuffers(1, &colorVBO);
	if (inNormal != -1) glDeleteBuffers(1, &normalVBO);
	if (inTexCoord != -1) glDeleteBuffers(1, &texCoordVBO);
	glDeleteBuffers(1, &triangleIndexVBO);
	glDeleteVertexArrays(1, &vao);

	glDeleteTextures(1, &colorTexId);  
	glDeleteTextures(1, &emiTexId);
	glDeleteTextures(1, &alphaTexId);
}

void generateRandomPoints()
{
	positionsParticles.resize(NUM_PARTICLES);
	velocitiesParticles.resize(NUM_PARTICLES);
	oldpositionsParticles.resize(NUM_PARTICLES);
	colorParticles.resize(NUM_PARTICLES);

	for (size_t i = 0; i < NUM_PARTICLES; i += 4)
	{
		float random = XMAX * (static_cast <float> (rand()) / (static_cast <float> (RAND_MAX)));
		glm::vec3 omega(0.0f, VXMAX * (static_cast <float> (rand()) / (static_cast <float> (RAND_MAX))), 0.0f);
		float r = 1.0f;
		float z = r * cos(random);
		float x = r * sin(random);
		glm::vec4 aux;
		aux.x = sqrt(r * r - (z * z));
		aux.y = YMIN + YMAX * static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
		aux.z = sqrt(r * r - (x * x));
		aux.w = PARTICLE_LIFETIME + (LIFETIME_MAX * static_cast <float> (rand()) / (static_cast <float> (RAND_MAX)));
		positionsParticles[i] = aux;
		glm::vec4 aux1 = glm::vec4(-aux.x, aux.y, aux.z, aux.w);
		positionsParticles[i+1] = aux1;
		glm::vec4 aux2 = glm::vec4(aux.x, aux.y, -aux.z, aux.w);
		positionsParticles[i+2] = aux2;
		glm::vec4 aux3 = glm::vec4(-aux.x, aux.y, -aux.z, aux.w);
		positionsParticles[i+3] = (aux3);

		glm::vec3 radio(aux.x, aux.y, aux.z);
		glm::vec3 radio1(aux1.x, aux1.y, aux1.z);
		glm::vec3 radio2(aux2.x, aux2.y, aux2.z);
		glm::vec3 radio3(aux3.x, aux3.y, aux3.z);
		glm::vec4 auxVel(glm::cross(omega, radio), 1.0);
		glm::vec4 auxVel1(glm::cross(omega, radio1), 1.0);
		glm::vec4 auxVel2(glm::cross(omega, radio2), 1.0);
		glm::vec4 auxVel3(glm::cross(omega, radio3), 1.0);
		velocitiesParticles[i] = auxVel;
		velocitiesParticles[i+1] = auxVel1;
		velocitiesParticles[i+2] = auxVel2;
		velocitiesParticles[i+3] = auxVel3;

		oldpositionsParticles[i] = glm::vec4(0.0f, 0.0f, -4.0f, 1.0f);
		oldpositionsParticles[i+1] = glm::vec4(0.0f, 0.0f, -4.0f, 1.0f);
		oldpositionsParticles[i+2] = glm::vec4(0.0f, 0.0f, -4.0f, 1.0f);
		oldpositionsParticles[i+3] = glm::vec4(0.0f, 0.0f, -4.0f, 1.0f);

		float randomC1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float randomC2= static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float randomC3= static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float randomAlpha= static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		colorParticles[i] = glm::vec4(randomC1, randomC2, randomC3, randomAlpha);
		colorParticles[i+1] = glm::vec4(randomC1, randomC3, randomC2, randomAlpha);
		colorParticles[i+2] = glm::vec4(randomC3, randomC2, randomC1, randomAlpha);
		colorParticles[i+3] = glm::vec4(randomC3, randomC1, randomC2, randomAlpha);
	}
	spawn = positionsParticles;
	spawnVel = velocitiesParticles;
	spawnOld = oldpositionsParticles;
}

void generateRandomPoints2()
{
	positionsParticles.resize(NUM_PARTICLES);
	velocitiesParticles.resize(NUM_PARTICLES);
	oldpositionsParticles.resize(NUM_PARTICLES);
	colorParticles.resize(NUM_PARTICLES);

	for (size_t i = 0; i < NUM_PARTICLES; i++)
	{
		float random = XMAX1 * (static_cast <float> (rand()) / (static_cast <float> (RAND_MAX)));
		glm::vec3 omega(0.0f, 0.4f + VXMAX1 * (static_cast <float> (rand()) / (static_cast <float> (RAND_MAX))), 0.0f);
		float r = RADIO + 0.2 * (static_cast <float> (rand()) / (static_cast <float> (RAND_MAX)));
		float z = r * cos(random);
		float x = r * sin(random);
		glm::vec4 aux;
		aux.x = sqrt(r * r - (z * z));
		aux.y = YMIN + YMAX * static_cast <float> (rand()) / (static_cast <float> (RAND_MAX));
		aux.z = sqrt(r * r - (x * x));
		aux.w = PARTICLE_LIFETIME + (LIFETIME_MAX * static_cast <float> (rand()) / (static_cast <float> (RAND_MAX)));
		positionsParticles[i] = aux;

		glm::vec3 radio(aux.x, aux.y, aux.z);
		glm::vec4 auxVel(glm::cross(omega, radio), 1.0);
		velocitiesParticles[i] = auxVel;

		oldpositionsParticles[i] = glm::vec4(0.0f, 0.0f, -4.0f, 1.0f);

		float randomC1 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float randomC2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float randomC3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float randomAlpha = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		colorParticles[i] = glm::vec4(randomC1, randomC2, randomC3, randomAlpha);
	}
	spawn = positionsParticles;
	spawnVel = velocitiesParticles;
	spawnOld = oldpositionsParticles;
}

void initShader(const char* vname, const char* fname, const char* gname, const char* tcname, const char* tename, struct program* program) 
{
	program->vshader = loadShader(vname, GL_VERTEX_SHADER);
	program->fshader = loadShader(fname, GL_FRAGMENT_SHADER);
	program->gshader = loadShader(gname, GL_GEOMETRY_SHADER);
	//program->tcshader = loadShader(tcname, GL_TESS_CONTROL_SHADER);
	//program->teshader = loadShader(tename, GL_TESS_EVALUATION_SHADER);

	program->program = glCreateProgram();
	glAttachShader(program->program, program->vshader);
	glAttachShader(program->program, program->fshader);
	glAttachShader(program->program, program->gshader);
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
	uProjectionMatrix = glGetUniformLocation(program->program, "proj");

	uAlphaTex = glGetUniformLocation(program->program, "alphaTex");

	uLightPosition = glGetUniformLocation(program->program, "lightPosition");
	uLightIntensity = glGetUniformLocation(program->program, "lightIntensity");

	inPos = glGetAttribLocation(program->program, "inPos");
	inColor = glGetAttribLocation(program->program, "inColor");
	inNormal = glGetAttribLocation(program->program, "inNormal");
	inTexCoord = glGetAttribLocation(program->program, "inTexCoord");

	if (uAlphaTex != -1) {
		glUniform1i(uAlphaTex, 0);
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

void firstStepVerlet()
{
	const float dt = 0.001;
	float G = 6.674 * pow(10, -11);
	float M1 = 100000000000.0;
	for (int i = 0; i < NUM_PARTICLES; i++)
	{
		glm::vec3 pos(positionsParticles[i]);
		glm::vec3 vel(velocitiesParticles[i]);

		float d = glm::distance( pos, glm::vec3(0.0f) );
		glm::vec3 acelGrav = -G * M1 * (1.0f / (d * d * d)) * pos;

		glm::vec3 pp = pos + vel * dt + (0.5f * dt * dt * acelGrav);
		glm::vec3 vp = (pp - pos) * (1.0f / dt);
		
		float w = PARTICLE_LIFETIME + (LIFETIME_MAX * static_cast <float> (rand()) / (static_cast <float> (RAND_MAX)));
		oldpositionsParticles[i] = glm::vec4(pos, w);
		positionsParticles[i] = glm::vec4(pp, w);
		velocitiesParticles[i] = glm::vec4(vp, 1.0f);
	}
	spawn = positionsParticles;
	spawnVel = velocitiesParticles;
	spawnOld = oldpositionsParticles;
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
	////////////////////////////////////////////    SSBO create, bind, etc   //////////////////////////////////////////////
	
	glGenBuffers(1, &posSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec4), &positionsParticles[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, posSSBO);


	glGenBuffers(1, &velSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec4), &velocitiesParticles[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, velSSBO);
	
	glGenBuffers(1, &oldPosSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, oldPosSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec4), &oldpositionsParticles[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, oldPosSSBO);

	glGenBuffers(1, &colorSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec4), &colorParticles[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, colorSSBO);

	glGenBuffers(1, &spawnSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, spawnSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec4), &positionsParticles[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, spawnSSBO);

	glGenBuffers(1, &spawnOldSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, spawnOldSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec4), &oldpositionsParticles[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, spawnOldSSBO);

	glGenBuffers(1, &spawnVelSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, spawnVelSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(glm::vec4), &velocitiesParticles[0], GL_STATIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, spawnVelSSBO);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	if (inPos != -1)
	{
		glBindBuffer(GL_ARRAY_BUFFER, posSSBO);
		glVertexAttribPointer(inPos, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inPos);
	}

	if (inPos != -1)
	{
		glBindBuffer(GL_ARRAY_BUFFER, colorSSBO);
		glVertexAttribPointer(inColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(inColor);
	}

	alphaTexId = loadTex("../img/2.png");
	
	//uAtractor = glGetUniformLocation(computeProgram->program, "atractor");
	uIncrementx = glGetUniformLocation(computeProgram->program, "add_x");
	uIncrementz = glGetUniformLocation(computeProgram->program, "add_z");
}

void initSortCompute(const char* computeName, struct computeProgram* computeProgram)
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
	uStage = glGetUniformLocation(computeProgram->program, "stage");
	uSubStage = glGetUniformLocation(computeProgram->program, "subStage");
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


	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//compute integrator shader
	glUseProgram(computePrograms[0].program);
	float x = RADIO * glm::cos(2 * 3.1415 * time);
	float z = RADIO * glm::sin(2 * 3.1415 * time);
	time += ATRAC_INCR;
	atractor = glm::vec3(x, 0.0, z);
	glUniform1f(uIncrementx, x);
	glUniform1f(uIncrementx, z);
	//glUniform3fv(uAtractor, 3 * sizeof(float), &atractor[0]);
	glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	if (BITONIC)
	{
		//compute sorting compute shader
		GLuint stage = std::log2(NUM_PARTICLES);
		GLuint subStage = 0;
		glUseProgram(computePrograms[1].program);
		for (size_t i = 0; i < stage; i++)
		{
			glUniform1ui(uStage, i);
			for (size_t j = 0; j < i + 1; j++)
			{
				glUniform1ui(uSubStage, j);
				glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			}
		}
	}

	//drawing shader
	glUseProgram(programs[0].program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, alphaTexId);

	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
	glBindVertexArray(0);

	glutSwapBuffers();
}

void renderParticles()
{
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
	//model = glm::mat4(1.0f);
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

