
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

//main함수에 paths를 넘기기 위해 전역변수 droppedFiles 설정.
std::vector<std::string> droppedFiles; 
void read_file(GLFWwindow* window, int count, const char** paths)
{
	droppedFiles.clear();
	for (int i = 0; i < count; i++)
	{
		droppedFiles.push_back(paths[i]);
	}
}

int main()
{
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* file_window = glfwCreateWindow(480, 360, "Please drop URDF file made by SOLIDWORKS", NULL, NULL);
	if (!file_window)
	{
		glfwTerminate();
		std::cout << "error!" << std::endl;
		return -1;
	}

	glfwMakeContextCurrent(file_window);
	// 파일 드롭 부분
	glfwSetDropCallback(file_window, read_file);
	while (!glfwWindowShouldClose(file_window))
	{
		//glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(file_window);
		glfwPollEvents();
		if (!droppedFiles.empty()) break;
	}
	glfwDestroyWindow(file_window);


	if (droppedFiles.empty()) // 파일 체크
	{
		std::cout << "File is empty." << std::endl;
		return -1;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, "wasd : move, space : up, ctrl : down", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGL();

	glViewport(0, 0, width, height);

	LoadURDF urdfLoad;
	std::list<std::string> stlnames;
	std::list<Eigen::Vector3d> xyzvectors, rpyvectors;
	std::vector<Vertex> vertices;
	std::vector<Triangle> triangles;
	const char* urdfname = droppedFiles[0].c_str();
	std::cout << urdfname << std::endl;

	//URDF로드 후 vertices에 넣기.
	urdfLoad.Load(urdfname, stlnames, xyzvectors, rpyvectors);
	SetURDF(stlnames, xyzvectors, rpyvectors, vertices);


	Shader shaderProgram("default.vert", "default.frag");

	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	glEnableVertexAttribArray(3);

	// Unbind
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);



	Shader lightShader("light.vert", "light.frag");
	VAO lightVAO;
	lightVAO.Bind();
	VBO lightVBO(lightVertices, sizeof(lightVertices));
	EBO lightEBO(lightIndices, sizeof(lightIndices));
	lightVAO.LinkAttrib(lightVBO, 0, 3, GL_FLOAT, 3 * sizeof(float), (void*)0);
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

	glEnable(GL_DEPTH_TEST);


	Camera camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));


	while (!glfwWindowShouldClose(window))
	{

		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		camera.Inputs(window);
		camera.updateMatrix(45.0f, 0.001f, 200.0f);

		shaderProgram.Activate();

		glUniform3f(glGetUniformLocation(shaderProgram.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);

		camera.Matrix(shaderProgram, "camMatrix");


		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		glBindVertexArray(0);


		lightShader.Activate();
		camera.Matrix(lightShader, "camMatrix");
		lightVAO.Bind();
		glDrawElements(GL_TRIANGLES, sizeof(lightIndices) / sizeof(int), GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}


	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);

	shaderProgram.Delete();
	lightVAO.Delete();
	lightVBO.Delete();
	lightEBO.Delete();
	lightShader.Delete();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}