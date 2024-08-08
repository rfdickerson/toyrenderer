//
// Created by Robert F. Dickerson on 7/4/24.
//

#include "camera.hpp"

using namespace obsidian;

Camera::Camera(
    glm::vec3 pos,
    glm::vec3 up,
    float yaw,
    float pitch)
: position(pos), front(glm::vec3(0.0f, 0.0f, -1.0f)), up(up),
yaw(yaw), pitch(pitch), fov(45.0f), aspectRatio(16.0/9.0f),
nearPlane(0.1f), farPlane(100.0f) {
	update_camera_vectors();
};

void Camera::update_camera_vectors() {
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix() const {
    glm::mat4 proj =  glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
	proj[1][1] *= -1;
	return proj;
}

void Camera::process_keyboard(CameraMovement direction, float deltaTime) {
    float velocity = 2.5f * deltaTime;
    if (direction == FORWARD) {
        position += front * velocity;
    }
    if (direction == BACKWARD) {
        position -= front * velocity;
    }
    if (direction == LEFT) {
        position -= glm::normalize(glm::cross(front, up)) * velocity;
    }
    if (direction == RIGHT) {
        position += glm::normalize(glm::cross(front, up)) * velocity;
    }
}

void Camera::process_mouse_movement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= 0.1f;
    yoffset *= 0.1f;

    yaw += xoffset;
    pitch += yoffset;

    if (constrainPitch) {
        if (pitch > 89.0f) {
            pitch = 89.0f;
        }
        if (pitch < -89.0f) {
            pitch = -89.0f;
        }
    }

	update_camera_vectors();
}

void Camera::process_mouse_scroll(float yoffset) {
    fov -= yoffset;
    if (fov < 1.0f) {
        fov = 1.0f;
    }
    if (fov > 45.0f) {
        fov = 45.0f;
    }
}