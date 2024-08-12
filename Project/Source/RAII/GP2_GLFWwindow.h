#ifndef GP2VKT_GP2_GLFWWINDOW_H_
#define GP2VKT_GP2_GLFWWINDOW_H_
// Includes

// Class Forward Declarations
struct GLFWwindow;


// RAII wrapper for GLFWwindow
class GP2_GLFWwindow final
{
public:
	// Constructors and Destructor
	GP2_GLFWwindow() = default;
	GP2_GLFWwindow(int width, int height, const char* title);
	~GP2_GLFWwindow();
	
	// Copy and Move semantics
	GP2_GLFWwindow(const GP2_GLFWwindow& other)					= delete;
	GP2_GLFWwindow& operator=(const GP2_GLFWwindow& other)		= delete;
	GP2_GLFWwindow(GP2_GLFWwindow&& other) noexcept				;
	GP2_GLFWwindow& operator=(GP2_GLFWwindow&& other) noexcept	;

	//---------------------------
	// Public Member Functions
	//---------------------------
	GLFWwindow* Get();


private:
	// Member variables
	GLFWwindow* m_pWindow{ nullptr };

	//---------------------------
	// Private Member Functions
	//---------------------------

};
#endif