#ifndef GP2VKT_GLFW_INIT_H_
#define GP2VKT_GLFW_INIT_H_

#include <GLFW/glfw3.h>

class GLFW final
{
public:
	GLFW() { glfwInit(); }
	~GLFW() { glfwTerminate(); }
};
#endif