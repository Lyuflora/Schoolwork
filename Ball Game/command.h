#pragma once
#ifndef COMMAND_H
#define COMMAND_H

void computeMatricesFromInputs();
bool towerMove(float deltaTime);

glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
glm::mat4 getModelMatrix_tower();
glm::mat4 getModelMatrix_ball();
#endif