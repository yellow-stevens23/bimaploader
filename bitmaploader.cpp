#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include <string.h>


#include <GL/glew.h>
#include <GLFW/glfw3.h>


using namespace std;

// window dimensions
int winWidth, winHeight;
// OpenGL global variables
GLuint VAO, VBO, shaderProgramme; 
GLenum format;

// BMP file header structs
#pragma pack(push, 1)	
	
struct BITMAPFileHeader
{
	unsigned char fileType1;
	unsigned char fileType2;
	uint32_t imageSize;
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t dataOffset; 
		
};
	
struct BITMAPInfoHeader
{
	uint32_t headerSize;
	int32_t width;
	int32_t height;
	uint16_t planes;
	uint16_t bitCount;
	uint32_t compression;
	uint32_t dataSize;
	int32_t horizontalResolution;
	int32_t verticalResolution;
	uint32_t coloursUsed;
	uint32_t importantColours; 
};
	
#pragma pack(pop)
	
BITMAPFileHeader* header;
BITMAPInfoHeader* info;

// vertex and fragement shader filenames
const char* pVSFilename ="Shaders/shader.ver";
const char* pFSFilename = "Shaders/shader.fra";

// bitmapfile name
char* bfName;

uint8_t* dataBuffer[] = {nullptr, nullptr}; 
uint8_t* imageData = nullptr; 

// function prototypes
bool loadBitmap(const char* bFilename);
bool readShaderFile(const char* pFileName, string& outFile);
static void addShader(GLuint shaderProgramme, const char* pShaderText, GLenum shaderType);
static void compileShaders();
void createObject();

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		cout << "Please enter a file to read" << endl;
		return -1;
	}
	
	bfName = argv[1];
	
	// initialise GLWF
	if(!glfwInit())
	{
		printf("GLWF Initialisation failed!");
		glfwTerminate();
		return -1;
	}
	
	// set up GLWFW windoq properties
	// OpenGl Version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	
	// core profile - no backwards compatibility
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// forward compatibility
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	
	if(!loadBitmap(bfName))
	{
		cout << "Dang it! See above for details" << endl; // I don't need to repeat myself this function gives out error messages
	}
	

	GLFWwindow *mainWindow = glfwCreateWindow(winWidth, winHeight, bfName, NULL, NULL);
	
	if (!mainWindow)
	{
		printf("GLFW window did not open, is it painted shut?");
		glfwTerminate(); // we'll just leave here and pretend this never happened.
		return -1;
	}
	
	// get buffer size information
	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);
	
	// Set context fpr GLEW to use
	glfwMakeContextCurrent(mainWindow);
	
	// allow modern extention features
	glewExperimental = GL_TRUE;
	
	if(glewInit() != GLEW_OK)
	{
		printf("GLEW initilisation failed!"); // what has been wrought?
		glfwDestroyWindow(mainWindow); // burn it, burn it with fire
		glfwTerminate();		// and run for the hills!
		return 1;	
	}
	
	// setup viewport size
	glViewport(0 ,0, bufferWidth, bufferHeight);
	
	createObject();
	compileShaders();
	
	float borderColour[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLuint textureID;
	
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColour);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, winWidth, winHeight, 0, format, GL_UNSIGNED_BYTE, imageData);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	
	// loop until window closed
	while(!glfwWindowShouldClose(mainWindow))
	{
		// get and handle user input events
		glfwPollEvents();
		
		
		// clear window
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glUseProgram(shaderProgramme);
		
		glBindVertexArray(VAO);
		
		glDrawArrays(GL_TRIANGLES, 0, 6); // Starting from vertex 0; 6 vertices total -> 2 triangle
		
		glBindVertexArray(0);
		
		glUseProgram(0);
		
		glfwSwapBuffers(mainWindow);
	}
	
	return 0;
}

bool loadBitmap(const char* bFilename)
{
	
	BITMAPFileHeader* header;
	BITMAPInfoHeader* info;
	
	std::ifstream file;
	file.open(bFilename, ios::binary);
	if (!file)
	{
		cout << "Image is missing from your hard drive!" << endl;
		return false;
	}
	
	dataBuffer[0] = new uint8_t[sizeof(BITMAPFileHeader)];
	dataBuffer[1] = new uint8_t[sizeof(BITMAPInfoHeader)];
	
	file.read((char*)dataBuffer[0], sizeof(BITMAPFileHeader));
	file.read((char*)dataBuffer[1], sizeof(BITMAPInfoHeader));
	
	header = (BITMAPFileHeader*) dataBuffer[0];
	info = (BITMAPInfoHeader*) dataBuffer[1];
	
	if((header->fileType1 != 'B') || (header->fileType2 != 'M'))
	{
		cout << "File \"" << bFilename << "\" isn't a bitmap file" << endl;
		return false;
	}
	
	imageData = new uint8_t[info->dataSize];

	// Go to where image data starts, then read in image data
	file.seekg(header->dataOffset);
	file.read((char*)imageData, info->dataSize);	
	
	int step;

	if(info->bitCount == 32) 
	{
		format = GL_RGBA;
		step = 4;
	}
	else if (info->bitCount == 24)
	{
		format = GL_RGB;
		step = 3;
	}
	else
	{	

		cout << "Whoa! How did you load THAT file? More importantly WHY did you load that file?!?!" << endl;
		return false;
	}
	
	uint8_t tempRGB = 0;
	for (unsigned long i=0; i < info->dataSize; i += step)
	{
		tempRGB = imageData[i];
		imageData[i] = imageData[i + 2];
		imageData[i+2] = tempRGB;
	} 
	
	winWidth = info->width;
	winHeight = info->height;
	
	return true;
}

bool readShaderFile(const char* pFileName, string& outFile)
{
    ifstream file(pFileName);
    
    bool retVal = false;
    
    if (file.is_open()) 
    {
        string line;
        while (getline(file, line)) 
        {
            outFile.append(line);
            outFile.append("\n");
        }
        
        file.close();
        
        retVal = true;
    }
    
    else 
    {
        cout << "Error: cannot open : "  << pFileName << endl;
    }
    
    return retVal;
}

static void addShader(GLuint shaderProgramme, const char* pShaderText, GLenum shaderType)
{
	GLuint shaderObj = glCreateShader(shaderType);
	if (!shaderObj)
	{
		cout << "Error creating shader type: " << shaderType << endl;
		return;
	}
	
	const GLchar* p[1];
	p[0] = pShaderText;
	GLint lengths[1];
	lengths[0] = strlen(pShaderText);
	glShaderSource(shaderObj, 1, p, lengths);
	
	glCompileShader(shaderObj);
	
	GLint success;
	glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[1024];
		glGetShaderInfoLog(shaderObj, sizeof(infoLog), NULL, infoLog);
		cout << "Error compiling shader type" <<  shaderType << ": " << infoLog << endl;
		return;
	}
	glAttachShader(shaderProgramme, shaderObj);
}

static void compileShaders()
{
	shaderProgramme = glCreateProgram();
	if (!shaderProgramme)
	{
		 cout << "Error creating shader program" << endl;
		 return;
		 
	}
	
	string vs, fs;
	
	if (!readShaderFile(pVSFilename, vs))
	{
		return;
	}
	
	if (!readShaderFile(pFSFilename, fs))
	{
		return;
	}
	
	addShader(shaderProgramme, vs.c_str(), GL_VERTEX_SHADER);
	addShader(shaderProgramme, fs.c_str(), GL_FRAGMENT_SHADER);
	
	GLint success;
	GLchar errorLog[1024] = { 0 };
	
	glLinkProgram(shaderProgramme);
	glGetProgramiv(shaderProgramme, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgramme, sizeof(errorLog), NULL, errorLog);
		cout << "Error linking shader programme: " << errorLog << endl;
		return;
	}
	
	glValidateProgram(shaderProgramme);
	glGetProgramiv(shaderProgramme, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgramme, sizeof(errorLog), NULL, errorLog);
		cout << "Invalid shader programme:" << errorLog << endl;
		return;
	}
	
}

void createObject()
{
	
	GLfloat vertices[] = {
	//	   x      y     z                s    t    
		-1.0f, 1.0f, 0.0f,		0.0f, 1.0f,	//	0
		-1.0f, -1.0f, 0.0f,		0.0f, 0.0f,	//	1
		1.0f, -1.0f, 0.0f,		1.0f, 0.0f,	//	2
		-1.0f, 1.0f, 0.0f,		0.0f, 1.0f,	//	3
		1.0f, 1.0f, 0.0f,		1.0f, 1.0f, 	//	4
		1.0f, -1.0f, 0.0f,		1.0f, 0.0f	// 	5
		
	};
	
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);
	glEnableVertexAttribArray(0); // position
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (char*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1); // texture
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

