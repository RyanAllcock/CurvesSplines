#include "camera.hpp"

// transformation methods

Transform::Transform(std::array<float, 3> const &p){
	rotateX = rotateY = 0;
	position = glm::vec3(p[0], p[1], p[2]);
	setRotate();
	setPosition();
}

void Transform::setRotate(){
	glm::mat4 pitch = glm::rotate(glm::mat4(1), rotateY, glm::vec3(-1, 0, 0));
	glm::mat4 yaw = glm::rotate(glm::mat4(1), rotateX, glm::vec3(0, 1, 0));
	rotation = pitch * yaw;
}

void Transform::setPosition(){
	placement = glm::translate(glm::mat4(1), -position);
}

glm::mat4 Transform::getView(){
	return rotation * placement;
}

// general operations

void Transform::rotate(float x, float y){
	rotateX += x;
	rotateY += y;
	rotateX = (rotateX > PI * 2 ? rotateX - PI * 2 : (rotateX < 0 ? rotateX + PI * 2 : rotateX));
	rotateY = (rotateY > PI / 2 ? PI / 2 : (rotateY < -PI / 2 ? -PI / 2 : rotateY));
	setRotate();
}

void Transform::lookAt(glm::vec3 const &target){
	glm::vec3 upGlobal = glm::vec3(0, 1.f, 0);
	glm::vec3 dir = glm::normalize(target - position);
	glm::vec3 right = glm::normalize(glm::cross(upGlobal, -dir));
	glm::vec3 up = glm::normalize(glm::cross(-dir, right));
	glm::vec3 forward = glm::normalize(glm::cross(upGlobal, right));
	rotateX = right.z < 0 ? PI * 2 - acos(right.x) : acos(right.x);
	rotateY = acos(glm::dot(forward, up) / (glm::length(forward) * glm::length(up))) - PI / 2.f;
	setRotate();
}

void Transform::move(glm::vec3 const &m){
	position += glm::vec3(transpose(rotation) * glm::vec4(m.x, m.y, -m.z, 1));
	setPosition();
}

void Transform::dolly(glm::vec3 const &pivot, float distance){
	glm::vec3 offset = glm::vec3(glm::transpose(rotation) * glm::vec4(0, 0, distance, 1));
	position = pivot + offset;
	setPosition();
}

// camera mode methods

CameraMode::CameraMode(Transform &t) : transform(t) {}

// free camera

CameraFree::CameraFree(Transform &t) : CameraMode(t) {}

glm::mat4 CameraFree::look(int cond, float x, float y){
	transform.rotate(x, y);
	return transform.getView();
}

glm::mat4 CameraFree::move(glm::vec3 const &m){
	transform.move(m);
	return transform.getView();
}

// focus camera

CameraFocus::CameraFocus(Transform &t, glm::vec3 const &target) : CameraMode(t) {
	transform.lookAt(target);
	centre = target;
	distance = glm::length(centre - transform.position);
}

glm::mat4 CameraFocus::look(int cond, float x, float y){
	if(cond){
		transform.rotate(x, y);
		transform.dolly(centre, distance); // rig camera at pivot
	}
	return transform.getView();
}

glm::mat4 CameraFocus::move(glm::vec3 const &m){
	return transform.getView();
}

// camera mode allocation

glm::mat4 Camera::setFree(){
	mode.reset(new CameraFree(transform));
	return transform.getView();
}

glm::mat4 Camera::setFocus(glm::vec3 const &target){
	mode.reset(new CameraFocus(transform, target));
	return transform.getView();
}

// camera input methods

Camera::Camera(std::array<float, 3> const &pos, float rSens, float mSens) : transform(pos), 
	rotateSensitivity(rSens), moveSensitivity(mSens), // settings
	mode(new CameraFree(transform)) { // mode
	projection = glm::mat4(1);
	view = transform.getView(); // view
}

void Camera::input(int select, int spin, int release, float motion[3], float turning[2]){
	view = mode->look(spin, rotateSensitivity * turning[0], rotateSensitivity * turning[1]);
	view = mode->move(moveSensitivity * glm::vec3(motion[0], motion[1], motion[2]));
	if(select) // set pivot focus mode
		view = setFocus(glm::vec3(0.f, 0.f, 0.f));
	if(release) // set free move mode
		view = setFree();
}

void Camera::insertUniforms(glm::mat4 &view, glm::mat4 &projection, glm::mat4 &rotation, glm::vec3 &position){
	view = this->view;
	projection = this->projection;
	rotation = transform.rotation;
	position = transform.position;
}

void Camera::setProjection(glm::mat4 &&p){
	projection = p;
};

std::array<float, 16> const Camera::getViewProjection(){
	std::array<float, 16> output;
	glm::mat4 compute = projection * view;
	for(int i = 0; i < 16; i++) output[i] = glm::value_ptr(compute)[i];
	return output;
}

// camera projection

CameraProjection::CameraProjection(float fov, float n, float f) : fieldOfView(fov), near(n), far(f) {
	state = std::make_unique<ProjectionPerspective>();
}

CameraProjection::CameraProjection(ProjectionType p, float fov, float n, float f) : CameraProjection(fov, n, f) {
	switch(p){
		case CameraPerspective:
			state = std::make_unique<ProjectionPerspective>();
			break;
		case CameraOrthographic:
			state = std::make_unique<ProjectionOrthographic>();
			break;
		default:
			state = std::make_unique<ProjectionPerspective>();
			break;
	};
}

void CameraProjection::set(Camera &c, float aspectRatio){
	c.setProjection(state->get(aspectRatio, fieldOfView, near, far));
}

std::array<float, 16> CameraProjection::get(float aspectRatio){
	std::array<float, 16> output;
	glm::mat4 projection = state->get(aspectRatio, fieldOfView, near, far);
	for(int i = 0; i < 16; i++) output[i] = glm::value_ptr(projection)[i];
	return output;
}

void CameraProjection::toggle(){
	state = state->pass();
}

glm::mat4 ProjectionPerspective::get(float aspectRatio, float fov, float near, float far) const {
	float fovr = (fov * PI / 180.f);
	float fovy = aspectRatio > 1 ? fovr : 2.f * atan(tan(fovr / 2.f) / aspectRatio);
	return glm::perspective(fovy, aspectRatio, near, far);
}

std::unique_ptr<ProjectionState> ProjectionPerspective::pass() const {
	return std::make_unique<ProjectionOrthographic>();
}

glm::mat4 ProjectionOrthographic::get(float aspectRatio, float fov, float near, float far) const {
	float xRatio = aspectRatio;
	float yRatio = 1.f;
	if(aspectRatio < 1){
		xRatio = 1.f;
		yRatio = 1.f / xRatio;
	}
	return glm::ortho(-xRatio, xRatio, -yRatio, yRatio, near, far);
}

std::unique_ptr<ProjectionState> ProjectionOrthographic::pass() const {
	return std::make_unique<ProjectionPerspective>();
}