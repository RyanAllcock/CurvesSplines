#ifndef HEADER_CAMERA
#define HEADER_CAMERA

#include <glm/glm.hpp> // matrix operations
#include <glm/ext.hpp> // matrix value references

#include <memory> // mode allocation

#ifndef PI
#define PI 3.14159
#endif

enum ProjectionType{
	CameraPerspective, 
	CameraOrthographic
};

struct Transform{
	
	// properties
	float rotateX, rotateY;
	glm::vec3 position;
	
	// matrices
	glm::mat4 rotation, placement;
	
	// general
	Transform(std::array<float, 3> const &p);
	
	// transformation
	void setRotate();
	void setPosition();
	glm::mat4 getView();
	
	// operations
	void rotate(float x, float y);
	void lookAt(glm::vec3 const &target);
	void move(glm::vec3 const &m);
	void dolly(glm::vec3 const &pivot, float distance);
};

struct CameraMode{
	Transform &transform;
	CameraMode(Transform &t);
	virtual glm::mat4 look(int cond, float x, float y) = 0;
	virtual glm::mat4 move(glm::vec3 const &m) = 0;
};

struct CameraFree : CameraMode{ // spin and move
	CameraFree(Transform &t);
	glm::mat4 look(int cond, float x, float y); // turn
	glm::mat4 move(glm::vec3 const &m); // fly
};

struct CameraFocus : CameraMode{ // spin and select
	glm::vec3 centre;
	float distance;
	CameraFocus(Transform &t, glm::vec3 const &target);
	glm::mat4 look(int cond, float x, float y); // pivot
	glm::mat4 move(glm::vec3 const &m); // nothing
};

class Camera{
	
	// settings
	float rotateSensitivity, moveSensitivity;
	
	// properties
	Transform transform;
	glm::mat4 view, projection;
	std::unique_ptr<CameraMode> mode;
	
	// modes
	glm::mat4 setFree();
	glm::mat4 setFocus(glm::vec3 const &target);
	
	// input
public:
	Camera(std::array<float, 3> const &pos, float rSens, float mSens);
	void input(int select, int spin, int release, float motion[3], float turning[2]);
	void insertUniforms(glm::mat4 &view, glm::mat4 &projection, glm::mat4 &rotation, glm::vec3 &position);
	void setProjection(glm::mat4 &&p);
	std::array<float, 16> const getViewProjection();
};

struct ProjectionState{
	virtual glm::mat4 get(float aspectRatio, float fov, float near, float far) const = 0;
	virtual std::unique_ptr<ProjectionState> pass() const = 0;
};

struct CameraProjection{
	float fieldOfView, near, far;
	std::unique_ptr<ProjectionState> state;
	CameraProjection(float fov, float n, float f);
	CameraProjection(ProjectionType p, float fov, float n, float f);
	void set(Camera &c, float aspectRatio);
	std::array<float, 16> get(float aspectRatio);
	void toggle();
};

struct ProjectionPerspective : ProjectionState{
	glm::mat4 get(float aspectRatio, float fov, float near, float far) const;
	std::unique_ptr<ProjectionState> pass() const;
};

struct ProjectionOrthographic : ProjectionState{
	glm::mat4 get(float aspectRatio, float fov, float near, float far) const;
	std::unique_ptr<ProjectionState> pass() const;
};

#endif