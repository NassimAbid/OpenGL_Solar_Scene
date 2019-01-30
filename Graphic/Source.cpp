// 02_Obj_Loader
// Version up to lab 2.4.

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
using namespace std;
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define GLM_ENABLE_EXPERIMENTAL

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtx/transform.hpp>

void errorCallbackGLFW(int error, const char* description);
void hintsGLFW();
void endProgram();
void render(double currentTime);
void update(double currentTime);
void setupRender();
void startup();
void onResizeCallback(GLFWwindow* window, int w, int h);
void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void onMouseMoveCallback(GLFWwindow* window, double x, double y);
void onMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset);
void debugGL();
void readTexture(string name, GLuint texture);

//added in myself
struct object;

static void APIENTRY openGLDebugCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const GLvoid* userParam);
string readShader(string name);
void  checkErrorShader(GLuint shader);
object ReadObj(string name, object cube);
void readTexture(string name, GLuint texture);


// VARIABLES
GLFWwindow*		window;
int				windowWidth = 1280;
int				windowHeight = 720;
bool			running = true;
glm::mat4		proj_matrix;
float fov = 45.0f;
glm::mat4 projection;


//construct a struct with all variables used for an object
struct object {
	GLuint          vao;
	GLuint          buffer[2];
	GLint           mv_location;
	GLint           proj_location;
	GLint			tex_location;
	GLuint			program;
	float           aspect;
	float			angleX = 0.0f;
	float			angleY = 0.0f;
	float			disp = 0.0;
	float			x = 0.0f;
	float			y = 0.0f;
	string			ID;
	glm::vec3 modelPositions;
	glm::mat4 modelMatrix;
	// extra variables for this example
	GLuint		matColor_location;
	GLuint		lightColor_location;
	// OBJ Variables

	std::vector < glm::vec3 > out_vertices;
	std::vector < glm::vec2 > out_uvs;
	std::vector < glm::vec3 > out_normals;
	GLuint*	texture;

};

vector	<glm::mat4> RockmodelInstances;
// Light
bool			movingLight = true;
glm::vec3		lightDisp = glm::vec3(5.0f, 2.0f, 3.0f);
glm::vec3		ia = glm::vec3(0.75f, 0.5f, 0.5f);
GLfloat			ka = 1.0f;
glm::vec3		id = glm::vec3(1.0f, 0.75f, 0.32f);
GLfloat			kd = 1.0f;
glm::vec3		is = glm::vec3(0.50f, 1.00f, 1.0f);
GLfloat			ks = 0.01f;

object Land;
object Water;
object Rock;
object lightModel;

object *objects[] = { &Water, &Land ,&Rock };
//gotten it to work from a structure format
int main()
{
	if (!glfwInit()) {							// Checking for GLFW
		cout << "Could not initialise GLFW...";
		return 0;
	}

	glfwSetErrorCallback(errorCallbackGLFW);	// Setup a function to catch and display all GLFW errors.

	hintsGLFW();								// Setup glfw with various hints.		

												// Start a window using GLFW
	string title = "My OpenGL Application";
	window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), NULL, NULL);
	if (!window) {								// Window or OpenGL context creation failed
		cout << "Could not initialise GLFW...";
		endProgram();
		return 0;
	}

	glfwMakeContextCurrent(window);				// making the OpenGL context current

												// Start GLEW (note: always initialise GLEW after creating your window context.)
	glewExperimental = GL_TRUE;					// hack: catching them all - forcing newest debug callback (glDebugMessageCallback)
	GLenum errGLEW = glewInit();
	if (GLEW_OK != errGLEW) {					// Problems starting GLEW?
		cout << "Could not initialise GLEW...";
		endProgram();
		return 0;
	}

	debugGL();									// Setup callback to catch openGL errors.	

												// Setup all the message loop callbacks.
	glfwSetWindowSizeCallback(window, onResizeCallback);		// Set callback for resize
	glfwSetKeyCallback(window, onKeyCallback);					// Set Callback for keys
	glfwSetMouseButtonCallback(window, onMouseButtonCallback);	// Set callback for mouse click
	glfwSetCursorPosCallback(window, onMouseMoveCallback);		// Set callback for mouse move
	glfwSetScrollCallback(window, onMouseWheelCallback);		// Set callback for mouse wheel.
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);	// Set mouse cursor.

	setupRender();								// setup some render variables.
	startup();									// Setup all necessary information for startup (aka. load texture, shaders, models, etc).

	do {										// run until the window is closed

		double currentTime = glfwGetTime();		// retrieve timelapse
		glfwPollEvents();						// poll callbacks
		update(currentTime);					// update (physics, animation, structures, etc)
		render(currentTime);					// call render function.

		glfwSwapBuffers(window);				// swap buffers (avoid flickering and tearing)


		running &= (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_RELEASE);	// exit if escape key pressed
		running &= (glfwWindowShouldClose(window) != GL_TRUE);
	} while (running);

	endProgram();			// Close and clean everything up...


	cout << "\nPress any key to continue...\n";
	cin.ignore(); cin.get(); // delay closing console to read debugging errors.

	return 0;
}

void errorCallbackGLFW(int error, const char* description) {
	cout << "Error GLFW: " << description << "\n";
}

void hintsGLFW() {
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);			// Create context in debug mode - for debug message callback
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
}

void endProgram() {
	glfwMakeContextCurrent(window);		// destroys window handler
	glfwTerminate();	// destroys all windows and releases resources.

}


void setupRender() {
	glfwSwapInterval(1);	// Ony render when synced (V SYNC)

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 0);
	glfwWindowHint(GLFW_STEREO, GL_FALSE);
}

void startup() {
	// Load Vertex shader as text from file
	objects[0]->ID = "water";
	objects[1]->ID = "land";
	objects[2]->ID = "rock";
	string vs_text = readShader("vs.glsl");  const char* vs_source = vs_text.c_str();
	string vs_text_instance = readShader("vs_instance_rock.glsl");  const char* vs_source_instance = vs_text_instance.c_str();

	// Load Fragment shader from file
	string fs_text = readShader("fs.glsl");  const char* fs_source = fs_text.c_str();

	// Compile and link shaders into program

	for (int i = 0; i <=2; i++) {
		objects[i]->program = glCreateProgram();

		if (i == 2) {
			GLuint vs = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vs, 1, &vs_source_instance, NULL);
			glCompileShader(vs);
			checkErrorShader(vs);

			GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fs, 1, &fs_source, NULL);
			glCompileShader(fs);
			checkErrorShader(fs);
			
			glAttachShader(objects[i]->program, vs);
			//this can be used to bake multiple textures.
			glAttachShader(objects[i]->program, fs);
		}
		else {

		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, &vs_source, NULL);
		glCompileShader(vs);
		checkErrorShader(vs);

		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, &fs_source, NULL);
		glCompileShader(fs);
		checkErrorShader(fs);
	
		glAttachShader(objects[i]->program, vs);
		//this can be used to bake multiple textures.
		glAttachShader(objects[i]->program, fs);
		}

		glLinkProgram(objects[i]->program);
		glValidateProgram(objects[i]->program);
		glUseProgram(objects[i]->program);

	
	}

	// Read my pacman OBJ file

	string file_Water = "doubletest.obj";
	string file_Land = "test.obj";
	string file_rock = "projRock.obj";

	//where the struct is called.

	Water = ReadObj(file_Water, Water);
	Land = ReadObj(file_Land, Land);
	Rock = ReadObj(file_rock, Rock);


	for (int i = 0; i <= 2; i++) {

		objects[i]->mv_location = glGetUniformLocation(objects[i]->program, "mv_matrix");
		objects[i]->proj_location = glGetUniformLocation(objects[i]->program, "proj_matrix");
		objects[i]->tex_location = glGetUniformLocation(objects[i]->program, "tex");
		objects[i]->lightColor_location = glGetUniformLocation(objects[i]->program, "ia");
		objects[i]->lightColor_location = glGetUniformLocation(objects[i]->program, "ka");
		glUniform1i(objects[i]->tex_location, 0);

		// Calculate proj_matrix for the first time.
		//should maybe change
		objects[i]->aspect = (float)windowWidth / (float)windowHeight;
		proj_matrix = glm::perspective(glm::radians(45.0f), objects[i]->aspect, 0.1f, 1000.0f);



		glGenVertexArrays(1, &objects[i]->vao);			// Create Vertex Array Object
		glBindVertexArray(objects[i]->vao);				// Bind VertexArrayObject

		glGenBuffers(3, objects[i]->buffer);			// Create new buffer

		glBindBuffer(GL_ARRAY_BUFFER, objects[i]->buffer[0]);	// Bind Buffer Vertex

		glBufferStorage(GL_ARRAY_BUFFER,
			objects[i]->out_vertices.size() * sizeof(glm::vec3),
			&objects[i]->out_vertices[0],
			GL_DYNAMIC_STORAGE_BIT);

		glBindBuffer(GL_ARRAY_BUFFER, objects[i]->buffer[1]);	// Bind Buffer uv coordinates
		glBufferStorage(GL_ARRAY_BUFFER,
			objects[i]->out_uvs.size() * sizeof(glm::vec2),
			&objects[i]->out_uvs[0],
			GL_DYNAMIC_STORAGE_BIT);

		glBindBuffer(GL_ARRAY_BUFFER, objects[i]->buffer[2]);	// Bind Buffer Normals
		glBufferStorage(GL_ARRAY_BUFFER, objects[i]->out_normals.size() * sizeof(glm::vec3), &objects[i]->out_normals[0], GL_DYNAMIC_STORAGE_BIT);

		// Atrib index , Size,Type,Normalised,Offset
		glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
		// Bind buffer to the vao and format. Binding Index. buffer ,offset ,Stride
		glBindVertexBuffer(0, objects[i]->buffer[0], 0, sizeof(GLfloat) * 3);
		glVertexAttribBinding(0, 0);	// Atrib Index, binding Index 
		glEnableVertexAttribArray(0);	// Enable Vertex Array Attribute
		// Atrib index, Size, Type, Normalised ,Offset		

		glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 0);
		// Bind buffer to the vao and format. Binding Index, buffer ,offset, Stride
		glBindVertexBuffer(1, objects[i]->buffer[1], 0, sizeof(GLfloat) * 2);
		glVertexAttribBinding(1, 1);	// Atrib Index, binding Index 
		glEnableVertexAttribArray(1);	// Enable Color Array Attribute

		glVertexAttribFormat(2, 3, GL_FLOAT, GL_FALSE, 0);		// Normals									
		glBindVertexBuffer(2, objects[i]->buffer[2], 0, sizeof(GLfloat) * 3);
		glVertexAttribBinding(2, 2);
		glEnableVertexAttribArray(2);


		
	}

	//WORKS UP TO HERE

	//--------------Light Model--------------------------
	lightModel.program = glCreateProgram();

	string vs_textLight = readShader("vs_light.glsl");  const char* vs_sourceLight = vs_textLight.c_str();
	GLuint vsLight = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vsLight, 1, &vs_sourceLight, NULL);
	glCompileShader(vsLight);
	checkErrorShader(vsLight);
	glAttachShader(lightModel.program, vsLight);

	string fs_textLight = readShader("fs_light.glsl");  const char* fs_sourceLight = fs_textLight.c_str();
	GLuint fsLight = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fsLight, 1, &fs_sourceLight, NULL);
	glCompileShader(fsLight);
	checkErrorShader(fsLight);
	glAttachShader(lightModel.program, fsLight);

	glLinkProgram(lightModel.program);

	lightModel = ReadObj("sphere.obj", lightModel);

	glGenVertexArrays(1, &lightModel.vao);			// Create Vertex Array Object
	glBindVertexArray(lightModel.vao);				// Bind VertexArrayObject

	glGenBuffers(3, lightModel.buffer);			// Create new buffers (Vertices, Texture Coordinates, Normals
	glBindBuffer(GL_ARRAY_BUFFER, lightModel.buffer[0]);	// Bind Buffer Vertex
	glBufferStorage(GL_ARRAY_BUFFER, lightModel.out_vertices.size() * sizeof(glm::vec3), &lightModel.out_vertices[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, lightModel.buffer[1]);	// Bind Buffer UV
	glBufferStorage(GL_ARRAY_BUFFER, lightModel.out_uvs.size() * sizeof(glm::vec2), &lightModel.out_uvs[0], GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, lightModel.buffer[2]);	// Bind Buffer Normals
	glBufferStorage(GL_ARRAY_BUFFER, lightModel.out_normals.size() * sizeof(glm::vec3), &lightModel.out_normals[0], GL_DYNAMIC_STORAGE_BIT);

	glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);		// Vertices									
	glBindVertexBuffer(0, lightModel.buffer[0], 0, sizeof(GLfloat) * 3);
	glVertexAttribBinding(0, 0);
	glEnableVertexAttribArray(0);

	glVertexAttribFormat(1, 2, GL_FLOAT, GL_FALSE, 0);			// UV									
	glBindVertexBuffer(1, lightModel.buffer[1], 0, sizeof(GLfloat) * 2);
	glVertexAttribBinding(1, 1);
	glEnableVertexAttribArray(1);

	glVertexAttribFormat(2, 3, GL_FLOAT, GL_FALSE, 0);			// Normals									
	glBindVertexBuffer(2, lightModel.buffer[2], 0, sizeof(GLfloat) * 3);
	glVertexAttribBinding(2, 2);
	glEnableVertexAttribArray(2);


	lightModel.mv_location = glGetUniformLocation(lightModel.program, "mv_matrix");
	lightModel.proj_location = glGetUniformLocation(lightModel.program, "proj_matrix");
	lightModel.tex_location = glGetUniformLocation(lightModel.program, "tex");


	//glFrontFace(GL_CCW);
	//glCullFace(GL_BACK);
	//glEnable(GL_CULL_FACE);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
}


void update(double currentTime) {

}

//this needs worked on to get a few objects working
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;
}
void render(double currentTime) {


	GLfloat radius = 20.0f;
	GLfloat camX = sin(currentTime) * radius;
	GLfloat camZ = cos(currentTime) * radius;




	glm::mat4 viewMatrix = glm::lookAt(glm::vec3(camX, 0.0, camZ),
		glm::vec3(0.0, 0.0, 0.0),
		glm::vec3(0.0, 1.0, 0.0));



	//window set up
	static const GLfloat silver[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glViewport(0, 0, windowWidth, windowHeight);
	glClearBufferfv(GL_COLOR, 0, silver);
	static const GLfloat one = 1.0f;
	glClearBufferfv(GL_DEPTH, 0, &one);
	// Enable blend
	glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//cube
	objects[1]->modelPositions
		= glm::vec3(-1.02f, -0.15f, 1.5f);

	objects[0]->modelPositions
		= glm::vec3(-1.0f, 0.0f, 1.5f);

	objects[2]->modelPositions
		= glm::vec3(-4.0f, 0.0f, 1.5f);


	glm::vec3 model_instance_Position[] = {
		glm::vec3(-4.0f,  0.0f,  0.0f),
		glm::vec3(6.5f, -0.5f, -0.5f),
		glm::vec3(1.0f,  0.0f, -1.0f),
		glm::vec3(0.5f, -1.0f, -1.5f),
		glm::vec3(-1.0f,  1.0f, -0.5f),
		glm::vec3(1.0f,  0.5f, -0.0f),
		glm::vec3(0.0f,  1.0f,  0.5f),
		glm::vec3(0.5f, -0.5f,  1.0f),
		glm::vec3(-1.0f,  0.0f,  1.5f),
		glm::vec3(-0.5f, -0.5f,  1.0f),
		glm::vec3(-1.0f, -1.0f,  0.5f),
		glm::vec3(-1.0f, -0.5f,  0.0f),
	};

	for (int i = 0; i <= 2; i++) {
		// Bind textures and samplers - using 0 as I know there is only one texture, only can hold one texture at once.
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, objects[i]->texture[0]);

		glUseProgram(objects[i]->program);
		glUniformMatrix4fv(objects[i]->proj_location, 1, GL_FALSE, &proj_matrix[0][0]);

		glUniform4f(glGetUniformLocation(objects[i]->program, "viewPosition"), camX, 0.0, camZ, 1.0);
		glUniform4f(glGetUniformLocation(objects[i]->program, "lightPosition"), lightDisp.x, lightDisp.y, lightDisp.z, 1.0);
		glUniform4f(glGetUniformLocation(objects[i]->program, "ia"), ia.r, ia.g, ia.b, 1.0);
		glUniform1f(glGetUniformLocation(objects[i]->program, "ka"), ka);
		glUniform4f(glGetUniformLocation(objects[i]->program, "id"), id.r, id.g, id.b, 1.0);
		glUniform1f(glGetUniformLocation(objects[i]->program, "kd"), 1.0f);
		glUniform4f(glGetUniformLocation(objects[i]->program, "is"), is.r, is.g, is.b, 1.0);
		glUniform1f(glGetUniformLocation(objects[i]->program, "ks"), 1.0f);
		glUniform1f(glGetUniformLocation(objects[i]->program, "shininess"), 32.0f);

		if (i == 2) {
			for (int j = 0; j <= 6; j++) {
				glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));// modelDisp.z));
				modelMatrix = glm::translate(modelMatrix, model_instance_Position[j]);
				modelMatrix = glm::scale(modelMatrix, glm::vec3(1.22f, 1.52f, 1.22f));
				RockmodelInstances.push_back(modelMatrix);
	
				
			}
			glUniformMatrix4fv(glGetUniformLocation(objects[i]->program, "model_array"), 6, GL_FALSE, &RockmodelInstances[0][0][0]);
			glUniformMatrix4fv(glGetUniformLocation(objects[i]->program, "view_matrix"), 1, GL_FALSE, &viewMatrix[0][0]);
			glEnableVertexAttribArray(0);
			glBindVertexArray(objects[i]->vao);
		
			glDrawArraysInstanced(GL_TRIANGLES, 0, objects[i]->out_vertices.size(), RockmodelInstances.size());
		}
		else {
		
			glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));// modelDisp.z));
			modelMatrix = glm::translate(modelMatrix, objects[i]->modelPositions);


			if (objects[i] == &Land)
				modelMatrix = glm::scale(modelMatrix, glm::vec3(4.22f, 3.52f, 3.22f));
			else
				modelMatrix = glm::scale(modelMatrix, glm::vec3(3.02f, 3.02f, 3.02f));

			glUniformMatrix4fv(glGetUniformLocation(objects[i]->program, "model_matrix"), 1, GL_FALSE, &modelMatrix[0][0]);
			glUniformMatrix4fv(glGetUniformLocation(objects[i]->program, "view_matrix"), 1, GL_FALSE, &viewMatrix[0][0]);
			glEnableVertexAttribArray(0);
			glBindVertexArray(objects[i]->vao);
			glDrawArrays(GL_TRIANGLES, 0, objects[i]->out_vertices.size());
			
		}




	}

	// ----------draw light------------
	glUseProgram(lightModel.program);
	glBindVertexArray(lightModel.vao);
	glUniformMatrix4fv(lightModel.proj_location, 1, GL_FALSE, &proj_matrix[0][0]);

	// Bind textures and samplers - using 0 as I know there is only one texture - need to extend.
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, lightModel.texture[0]);
	glUniform1i(lightModel.tex_location, 1);

	glm::mat4 modelMatrixLight = glm::translate(glm::mat4(1.0f), glm::vec3(lightDisp.x, lightDisp.y, lightDisp.z));
	modelMatrixLight = glm::scale(modelMatrixLight, glm::vec3(0.9f, 0.9f, 0.9f));
	glm::mat4 mv_matrixLight = viewMatrix * modelMatrixLight;

	glUniformMatrix4fv(lightModel.mv_location, 1, GL_FALSE, &mv_matrixLight[0][0]);
	glEnableVertexAttribArray(0);
	glBindVertexArray(lightModel.vao);
	glDrawArrays(GL_TRIANGLES, 0, lightModel.out_vertices.size());


}

void onResizeCallback(GLFWwindow* window, int w, int h) {
	windowWidth = w;
	windowHeight = h;
	//for (int i = 0; i <= sizeof(object) / sizeof(object[1]); i++) {
	//	objects[i]->aspect = (float)w / (float)h;
	//	proj_matrix = glm::perspective(50.0f, objects[i]->aspect, 0.1f, 1000.0f);

	//}
}

void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (key == GLFW_KEY_LEFT) objects[0]->angleY += 0.05f;
	if (key == GLFW_KEY_RIGHT) objects[0]->angleY -= 0.05f;

	if (key == GLFW_KEY_UP) objects[0]->angleX += 0.05f;
	if (key == GLFW_KEY_DOWN) objects[0]->angleX -= 0.05f;

	if (key == GLFW_KEY_A) objects[1]->angleY += 0.05f;
	if (key == GLFW_KEY_A) objects[1]->angleY -= 0.05f;

	if (key == GLFW_KEY_W) objects[1]->angleX += 0.05f;
	if (key == GLFW_KEY_D) objects[1]->angleX -= 0.05f;

	if (key == GLFW_KEY_A) objects[2]->angleY += 0.05f;
	if (key == GLFW_KEY_A) objects[2]->angleY -= 0.05f;

	if (key == GLFW_KEY_W) objects[2]->angleX += 0.05f;
	if (key == GLFW_KEY_D) objects[2]->angleX -= 0.05f;

	if (key == GLFW_KEY_KP_ADD) objects[0]->disp += 0.10f;
	if (key == GLFW_KEY_KP_SUBTRACT) objects[0]->disp -= 0.10f;



}

void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {

}

void onMouseMoveCallback(GLFWwindow* window, double x, double y) {
	int mouseX = static_cast<int>(x);
	int mouseY = static_cast<int>(y);


}

static void onMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset) {

	if (fov >= 1.0f && fov <= 45.0f)
		fov -= yoffset;
	if (fov <= 1.0f)
		fov = 1.0f;
	if (fov >= 45.0f)
		fov = 45.0f;


}

void debugGL() {
	//Output some debugging information
	cout << "VENDOR: " << (char *)glGetString(GL_VENDOR) << endl;
	cout << "VERSION: " << (char *)glGetString(GL_VERSION) << endl;
	cout << "RENDERER: " << (char *)glGetString(GL_RENDERER) << endl;

	// Enable Opengl Debug
	//glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback((GLDEBUGPROC)openGLDebugCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);
}

static void APIENTRY openGLDebugCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const GLvoid* userParam) {

	cout << "---------------------opengl-callback------------" << endl;
	cout << "Message: " << message << endl;
	cout << "type: ";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		cout << "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		cout << "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		cout << "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		cout << "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		cout << "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_OTHER:
		cout << "OTHER";
		break;
	}
	cout << " --- ";

	cout << "id: " << id << " --- ";
	cout << "severity: ";
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:
		cout << "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		cout << "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		cout << "HIGH";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		cout << "NOTIFICATION";
	}
	cout << endl;
	cout << "-----------------------------------------" << endl;
}

string readShader(string name) {
	string vs_text;
	std::ifstream vs_file(name);

	string vs_line;
	if (vs_file.is_open()) {

		while (getline(vs_file, vs_line)) {
			vs_text += vs_line;
			vs_text += '\n';
		}
		vs_file.close();
	}
	return vs_text;
}

void  checkErrorShader(GLuint shader) {
	// Get log lenght
	GLint maxLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

	// Init a string for it
	std::vector<GLchar> errorLog(maxLength);

	if (maxLength > 1) {
		// Get the log file
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

		cout << "--------------Shader compilation error-------------\n";
		cout << errorLog.data();
	}

}

object ReadObj(string name, object obj) {

	cout << "Loading model " << name << "\n";

	std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
	std::vector< string > materials, textures;
	std::vector< glm::vec3 > obj_vertices;
	std::vector< glm::vec2 > obj_uvs;
	std::vector< glm::vec3 > obj_normals;

	std::ifstream dataFile(name);

	string rawData;		// store the raw data.
	int countLines = 0;
	if (dataFile.is_open()) {
		string dataLineRaw;
		while (getline(dataFile, dataLineRaw)) {
			rawData += dataLineRaw;
			rawData += "\n";
			countLines++;
		}
		dataFile.close();
	}

	cout << "Finished reading model file " << name << "\n";

	istringstream rawDataStream(rawData);
	string dataLine;
	int linesDone = 0;
	while (std::getline(rawDataStream, dataLine)) {
		if (dataLine.find("v ") != string::npos) {	// does this line have a vector?
			glm::vec3 vertex;

			int foundStart = dataLine.find(" ");  int foundEnd = dataLine.find(" ", foundStart + 1);
			vertex.x = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
			vertex.y = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
			vertex.z = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			obj_vertices.push_back(vertex);
		}
		else if (dataLine.find("vt ") != string::npos) {	// does this line have a uv coordinates?
			glm::vec2 uv;

			int foundStart = dataLine.find(" ");  int foundEnd = dataLine.find(" ", foundStart + 1);
			uv.x = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
			uv.y = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			obj_uvs.push_back(uv);
		}
		else if (dataLine.find("vn ") != string::npos) { // does this line have a normal coordinates?
			glm::vec3 normal;

			int foundStart = dataLine.find(" ");  int foundEnd = dataLine.find(" ", foundStart + 1);
			normal.x = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
			normal.y = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
			normal.z = stof(dataLine.substr(foundStart, foundEnd - foundStart));

			obj_normals.push_back(normal);
		}
		else if (dataLine.find("f ") != string::npos) { // does this line defines a triangle face?
			string parts[3];

			int foundStart = dataLine.find(" ");  int foundEnd = dataLine.find(" ", foundStart + 1);
			parts[0] = dataLine.substr(foundStart + 1, foundEnd - foundStart - 1);

			foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
			parts[1] = dataLine.substr(foundStart + 1, foundEnd - foundStart - 1);

			foundStart = foundEnd; foundEnd = dataLine.find(" ", foundStart + 1);
			parts[2] = dataLine.substr(foundStart + 1, foundEnd - foundStart - 1);

			for (int i = 0; i < 3; i++) {		// for each part

				vertexIndices.push_back(stoul(parts[i].substr(0, parts[i].find("/"))));

				int firstSlash = parts[i].find("/"); int secondSlash = parts[i].find("/", firstSlash + 1);


				if ((firstSlash + 1) != (secondSlash)) {	// there are texture coordinates.
					uvIndices.push_back(stoul(parts[i].substr(firstSlash + 1, secondSlash - firstSlash + 1)));
				}


				normalIndices.push_back(stoul(parts[i].substr(secondSlash + 1)));

			}
		}
		else if (dataLine.find("mtllib ") != string::npos) { // does this object have a material?
			materials.push_back(dataLine.substr(dataLine.find(" ") + 1));
		}

		linesDone++;

		if (linesDone % 50000 == 0) {
			cout << "Parsed " << linesDone << " of " << countLines << " from model...\n";
		}

	}

	for (unsigned int i = 0; i < vertexIndices.size(); i += 3) {
		obj.out_vertices.push_back(obj_vertices[vertexIndices[i + 2] - 1]);
		obj.out_normals.push_back(obj_normals[normalIndices[i + 2] - 1]);
		obj.out_uvs.push_back(obj_uvs[uvIndices[i + 2] - 1]);
		obj.out_vertices.push_back(obj_vertices[vertexIndices[i + 1] - 1]);
		obj.out_normals.push_back(obj_normals[normalIndices[i + 1] - 1]);
		obj.out_uvs.push_back(obj_uvs[uvIndices[i + 1] - 1]);
		obj.out_vertices.push_back(obj_vertices[vertexIndices[i] - 1]);
		obj.out_normals.push_back(obj_normals[normalIndices[i] - 1]);
		obj.out_uvs.push_back(obj_uvs[uvIndices[i + 0] - 1]);
	}
	// Load Materials if there is one
	for (unsigned int i = 0; i < materials.size(); i++) {

		std::ifstream dataFileMat(materials[i]);

		string rawDataMat;		// store the raw data.
		int countLinesMat = 0;
		if (dataFileMat.is_open()) {
			string dataLineRawMat;
			while (getline(dataFileMat, dataLineRawMat)) {
				rawDataMat += dataLineRawMat;

				rawDataMat += "\n";
			}
			dataFileMat.close();
		}

		istringstream rawDataStreamMat(rawDataMat);
		string dataLineMat;
		while (std::getline(rawDataStreamMat, dataLineMat)) {
			if (dataLineMat.find("map_Kd ") != string::npos) {	// does this line have a texture map?
				textures.push_back(dataLineMat.substr(dataLineMat.find(" ") + 1));
			}
		}
	}

	obj.texture = new GLuint[textures.size()];		// Warning possible memory leak here - there is a new here, where is your delete?

	glGenTextures(textures.size(), obj.texture);

	for (int i = 0; i < textures.size(); i++) {
		readTexture(textures[i], obj.texture[i]);
	}

	cout << "done";
	return obj;
}


void readTexture(string name, GLuint textureName) {

	// Flip images as OpenGL expects 0.0 coordinates on the y-axis to be at the bottom of the image.
	stbi_set_flip_vertically_on_load(true);

	// Load image Information.
	int iWidth, iHeight, iChannels;
	unsigned char *iData = stbi_load(name.c_str(), &iWidth, &iHeight, &iChannels, 0);

	// Load and create a texture 
	glBindTexture(GL_TEXTURE_2D, textureName); // All upcoming operations now have effect on this texture object

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, iWidth, iHeight);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, iWidth, iHeight, GL_RGBA, GL_UNSIGNED_BYTE, iData);

	// This only works for 2D Textures...
	// Set the texture wrapping parameters (next lecture)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// Set texture filtering parameters (next lecture)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Generate mipmaps (next lecture)
	glGenerateMipmap(GL_TEXTURE_2D);

}