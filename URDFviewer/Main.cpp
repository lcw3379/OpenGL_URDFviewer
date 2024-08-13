
#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<list>
#include<vector>
#include<Eigen/Dense>

#include"shaderClass.h"
#include"VAO.h"
#include"VBO.h"
#include"EBO.h"
#include"Camera.h"
#include "LoadURDF.h"



const unsigned int width = 800;
const unsigned int height = 800;
GLfloat lightVertices[] =
{ //     COORDINATES     //
	-0.1f, -0.1f,  0.1f,
	-0.1f, -0.1f, -0.1f,
	 0.1f, -0.1f, -0.1f,
	 0.1f, -0.1f,  0.1f,
	-0.1f,  0.1f,  0.1f,
	-0.1f,  0.1f, -0.1f,
	 0.1f,  0.1f, -0.1f,
	 0.1f,  0.1f,  0.1f
};

GLuint lightIndices[] =
{
	0, 1, 2,
	0, 2, 3,
	0, 4, 7,
	0, 7, 3,
	3, 7, 6,
	3, 6, 2,
	2, 6, 5,
	2, 5, 1,
	1, 5, 4,
	1, 4, 0,
	4, 5, 6,
	4, 6, 7
};

struct Vector3 {
	float x, y, z;
};

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
};

struct Triangle {
	glm::vec3 normal;
	glm::vec3 vertex[3];
};

Eigen::Matrix4d CalculateTransformationMatrix(const Eigen::Vector3d& xyz, const Eigen::Vector3d& rpy) {
	// Create translation matrix
	Eigen::Matrix4d translation = Eigen::Matrix4d::Identity();
	translation(0, 3) = xyz.x();
	translation(1, 3) = xyz.y();
	translation(2, 3) = xyz.z();

	// Create rotation matrices using Roll-Pitch-Yaw angles
	Eigen::AngleAxisd rollAngle(rpy.x(), Eigen::Vector3d::UnitX());
	Eigen::AngleAxisd pitchAngle(rpy.y(), Eigen::Vector3d::UnitY());
	Eigen::AngleAxisd yawAngle(rpy.z(), Eigen::Vector3d::UnitZ());

	// Combine rotations in ZYX order (yaw, pitch, roll)
	Eigen::Quaternion<double> q = yawAngle * pitchAngle * rollAngle;
	Eigen::Matrix3d rotation = q.matrix();

	// Combine translation and rotation into a single transformation matrix
	Eigen::Matrix4d transformation = Eigen::Matrix4d::Identity();
	transformation.block<3, 3>(0, 0) = rotation;
	transformation.block<3, 1>(0, 3) = translation.block<3, 1>(0, 3);

	return transformation;
}


bool LoadSTL(const std::string& filename, std::vector<Triangle>& triangles) {
	std::ifstream stlFile(filename, std::ios::binary);
	if (!stlFile) {
		std::cerr << "Cannot open " << filename << std::endl;
		return false;
	}

	stlFile.seekg(80, std::ios::beg);

	unsigned int numTriangles = 0;
	stlFile.read(reinterpret_cast<char*>(&numTriangles), sizeof(unsigned int));

	triangles.resize(numTriangles);

	for (Triangle& tri : triangles) {
		// Read normal
		stlFile.read(reinterpret_cast<char*>(&tri.normal), sizeof(Vector3));
		// Read vertices
		for (int i = 0; i < 3; ++i) {
			stlFile.read(reinterpret_cast<char*>(&tri.vertex[i]), sizeof(Vector3));
		}
		// Skip attribute byte count
		stlFile.seekg(2, std::ios::cur);
	}

	return true;
}
float scrollValue = 0.0f; // 스크롤 값
GLFWwindow* scrollWindow = nullptr; // 스크롤 바가 있는 새 창
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (window == scrollWindow && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		// xpos를 기반으로 scrollValue 업데이트
		scrollValue = xpos;

		// 새 창의 제목 업데이트
		std::string title = "Scroll Value: " + std::to_string(scrollValue);
		glfwSetWindowTitle(window, title.c_str());
	}
}

void SetURDF(std::list<std::string>& filename, std::list<Eigen::Vector3d> xyzvectors, std::list<Eigen::Vector3d> rpyvectors, std::vector<Vertex>& vertices)
{
	Eigen::Vector3d zero2l1 = { 0, 0, 0 };
	xyzvectors.push_front(zero2l1);
	rpyvectors.push_front(zero2l1);

	auto it = xyzvectors.begin();
	auto it2 = rpyvectors.begin();
	int a = 0;
	int c = 0;
	Eigen::Matrix4d PrevT = Eigen::Matrix4d::Identity();
	std::vector<Triangle> triangles;

	for (const auto& str : filename) {
		Eigen::Vector3d xyz = *it;
		Eigen::Vector3d rpy = *it2;

		LoadSTL(str, triangles);

		Eigen::Matrix4d T = CalculateTransformationMatrix(xyz, rpy);
		Eigen::Matrix4d T_global = PrevT * T;

		for (const Triangle& tri : triangles) {;
		Eigen::Vector4d normalpoint = { tri.normal.x,tri.normal.y ,tri.normal.z ,1 };
		normalpoint = T_global * normalpoint;
			for (int i = 0; i <= 2; i++) {
				Eigen::Vector4d point = { tri.vertex[i].x,tri.vertex[i].y ,tri.vertex[i].z ,1 };
				point = T_global * point;
				Vertex vertex;
				vertex.Position = { point[0], point[1], point[2] };
				vertex.Normal = { normalpoint[0],normalpoint[1] ,normalpoint [2]};
				vertices.push_back(vertex);
			}
		}
		++it;
		++it2;
		PrevT = T_global;
	}
}




int main()
{
	// Initialize GLFW
	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	// In this case we are using OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// Tell GLFW we are using the CORE profile
	// So that means we only have the modern functions
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create a GLFWwindow object of 800 by 800 pixels, naming it "YoutubeOpenGL"
	GLFWwindow* window = glfwCreateWindow(width, height, "URDFviewer", NULL, NULL);

	// Error check if the window fails to create
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	// Introduce the window into the current context
	glfwMakeContextCurrent(window);
	scrollWindow = glfwCreateWindow(640, 100, "Scroll Bar Window", NULL, NULL);
	if (!scrollWindow) {
		glfwTerminate();
		return -1;
	}

	glfwSetMouseButtonCallback(scrollWindow, mouse_button_callback);
	//Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	// In this case the viewport goes from x = 0, y = 0, to x = 800, y = 800
	glViewport(0, 0, width, height);



	// Generates Shader object using shaders default.vert and default.frag
	Shader shaderProgram("default.vert", "default.frag");
	// Generates Vertex Array Object and binds it

	LoadURDF urdfLoad;
    std::list<std::string> stlnames;
    std::list<Eigen::Vector3d> xyzvectors, rpyvectors;
	std::vector<Vertex> vertices;
	std::vector<Triangle> triangles;
	const char* urdfname = "D:/robotarm_urdf7/urdf/robotarm_urdf7.urdf";

    urdfLoad.Load(urdfname,stlnames,xyzvectors,rpyvectors);

	SetURDF(stlnames, xyzvectors, rpyvectors,vertices);

	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
	glEnableVertexAttribArray(0);
	// Normal attribute
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	glEnableVertexAttribArray(3);

	// Unbind the VAO
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);



	// Shader for light cube
	Shader lightShader("light.vert", "light.frag");
	// Generates Vertex Array Object and binds it
	VAO lightVAO;
	lightVAO.Bind();
	// Generates Vertex Buffer Object and links it to vertices
	VBO lightVBO(lightVertices, sizeof(lightVertices));
	// Generates Element Buffer Object and links it to indices
	EBO lightEBO(lightIndices, sizeof(lightIndices));
	// Links VBO attributes such as coordinates and colors to VAO
	lightVAO.LinkAttrib(lightVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
	// Unbind all to prevent accidentally modifying them
	lightVAO.Unbind();
	lightVBO.Unbind();
	lightEBO.Unbind();



	glm::vec4 lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);
	glm::mat4 lightModel = glm::mat4(1.0f);
	lightModel = glm::translate(lightModel, lightPos);

	glm::vec3 pyramidPos = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::mat4 pyramidModel = glm::mat4(1.0f);
	pyramidModel = glm::translate(pyramidModel, pyramidPos);






	lightShader.Activate();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.ID, "model"), 1, GL_FALSE, glm::value_ptr(lightModel));
	glUniform4f(glGetUniformLocation(lightShader.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	shaderProgram.Activate();
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram.ID, "model"), 1, GL_FALSE, glm::value_ptr(pyramidModel));
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);



	// Enables the Depth Buffer
	glEnable(GL_DEPTH_TEST);

	// Creates camera object
	Camera camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));

	// Main while loop
	while (!glfwWindowShouldClose(window)&& !glfwWindowShouldClose(scrollWindow))
	{
		// Specify the color of the background
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		// Clean the back buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Handles camera inputs
		camera.Inputs(window);
		camera.updateMatrix(45.0f, 0.001f, 200.0f);



		shaderProgram.Activate();

		glUniform3f(glGetUniformLocation(shaderProgram.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);

		camera.Matrix(shaderProgram, "camMatrix");


		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		glBindVertexArray(0);



		lightShader.Activate();
		// Export the camMatrix to the Vertex Shader of the light cube
		camera.Matrix(lightShader, "camMatrix");
		lightVAO.Bind();
		glDrawElements(GL_TRIANGLES, sizeof(lightIndices) / sizeof(int), GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);




		glfwPollEvents();
	}



	// Delete all the objects we've created
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	shaderProgram.Delete();
	lightVAO.Delete();
	lightVBO.Delete();
	lightEBO.Delete();
	lightShader.Delete();
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}