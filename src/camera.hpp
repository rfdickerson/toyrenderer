//
#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace obsidian
{
enum CameraMovement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

class Camera
{
  public:
	explicit Camera(glm::vec3 pos = glm::vec3(0.0f, 0.0f, 3.0f),
	       glm::vec3 up  = glm::vec3(0.0f, 1.0f, 0.0f),
	       float yaw = -90.0f, float pitch = 0.0f);

	glm::mat4 getViewMatrix() const;
	glm::mat4 getProjectionMatrix() const;

	void process_keyboard(CameraMovement direction, float deltaTime);

	void process_mouse_movement(float xoffset, float yoffset, bool constrainPitch = true);

	void process_mouse_scroll(float yoffset);

	void update_camera_vectors();

	void look_at(glm::vec3 target);

  public:

	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	float     yaw;
	float     pitch;
	float     fov;
	float     aspectRatio;
	float     nearPlane;
	float     farPlane;
};
}; // namespace obsidian

