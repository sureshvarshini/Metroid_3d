#include "camera.h"
#include <GL/freeglut.h>
#include <iostream>
#include <string>
#include <stdio.h>

#include "../maths_funcs.h"

static int MARGIN = 10;
static float EDGE_STEP = 1.0f;

Camera::Camera(int WindowWidth, int WindowHeight)
{
    m_windowWidth = WindowWidth;
    m_windowHeight = WindowHeight;
    m_pos = vec3(0.0f, 0.0f, 0.0f);
    m_target = vec3(0.0f, 0.0f, 1.0f);
    m_up = vec3(0.0f, 1.0f, 0.0f);

    Init();
}


Camera::Camera(int WindowWidth, int WindowHeight, const vec3& Pos, const vec3& Target, const vec3& Up)
{
    m_windowWidth = WindowWidth;
    m_windowHeight = WindowHeight;
    m_pos = Pos;

    m_target = Target;
    m_target =normalise(m_target);

    m_up = Up;
    m_up = normalise(m_up);

    Init();
}


void Camera::Init()
{
    vec3 HTarget(m_target.x, 0.0, m_target.z);
    HTarget = normalise(HTarget);

    float Angle = ToDegree(asin(abs(HTarget.z)));

    if (HTarget.z >= 0.0f)
    {
        if (HTarget.x >= 0.0f)
        {
            m_AngleH = 360.0f - Angle;
        }
        else
        {
            m_AngleH = 180.0f + Angle;
        }
    }
    else
    {
        if (HTarget.x >= 0.0f)
        {
            m_AngleH = Angle;
        }
        else
        {
            m_AngleH = 180.0f - Angle;
        }
    }

    m_AngleV = -ToDegree(asin(m_target.y));

    m_OnUpperEdge = false;
    m_OnLowerEdge = false;
    m_OnLeftEdge = false;
    m_OnRightEdge = false;
    m_mousePos.x = m_windowWidth / 2;
    m_mousePos.y = m_windowHeight / 2;
}



void Camera::SetPosition(float x, float y, float z)
{
    m_pos.x = x;
    m_pos.y = y;
    m_pos.z = z;
}


void Camera::OnKeyboard(unsigned char Key)
{
    switch (Key) {

    case GLUT_KEY_UP:
        m_pos += (m_target * m_speed);
        break;

    case GLUT_KEY_DOWN:
        m_pos -= (m_target * m_speed);
        break;

    case GLUT_KEY_LEFT:
    {
        vec3 Left = cross(m_target, m_up);
        Left = normalise(Left);
        Left *= m_speed;
        m_pos += Left;
    }
    break;

    case GLUT_KEY_RIGHT:
    {
        vec3 Right = cross(m_up, m_target);
        Right = normalise(Right);
        Right *= m_speed;
        m_pos += Right;
    }
    break;

    case GLUT_KEY_PAGE_UP:
        m_pos.y += m_speed;
        break;

    case GLUT_KEY_PAGE_DOWN:
        m_pos.y -= m_speed;
        break;

    case '+':
        m_speed += 0.1f;
        printf("Speed changed to %f\n", m_speed);
        break;

    case '-':
        m_speed -= 0.1f;
        if (m_speed < 0.1f) {
            m_speed = 0.1f;
        }
        printf("Speed changed to %f\n", m_speed);
        break;
    }
}


void Camera::OnMouse(int x, int y)
{
    int DeltaX = x - m_mousePos.x;
    int DeltaY = y - m_mousePos.y;

    m_mousePos.x = x;
    m_mousePos.y = y;

    m_AngleH += (float)DeltaX / 20.0f;
    m_AngleV += (float)DeltaY / 50.0f;

    if (DeltaX == 0) {
        if (x <= MARGIN) {
            m_OnLeftEdge = true;
        }
        else if (x >= (m_windowWidth - MARGIN)) {
            m_OnRightEdge = true;
        }
    }
    else {
        m_OnLeftEdge = false;
        m_OnRightEdge = false;
    }

    if (DeltaY == 0) {
        if (y <= MARGIN) {
            m_OnUpperEdge = true;
        }
        else if (y >= (m_windowHeight - MARGIN)) {
            m_OnLowerEdge = true;
        }
    }
    else {
        m_OnUpperEdge = false;
        m_OnLowerEdge = false;
    }

    Update();
}


void Camera::OnRender()
{
    bool ShouldUpdate = false;

    if (m_OnLeftEdge) {
        m_AngleH -= EDGE_STEP;
        ShouldUpdate = true;
    }
    else if (m_OnRightEdge) {
        m_AngleH += EDGE_STEP;
        ShouldUpdate = true;
    }

    if (m_OnUpperEdge) {
        if (m_AngleV > -90.0f) {
            m_AngleV -= EDGE_STEP;
            ShouldUpdate = true;
        }
    }
    else if (m_OnLowerEdge) {
        if (m_AngleV < 90.0f) {
            m_AngleV += EDGE_STEP;
            ShouldUpdate = true;
        }
    }

    if (ShouldUpdate) {
        Update();
    }
}

void Camera::Update()
{
    vec3 Yaxis(0.0f, 1.0f, 0.0f);

    // Rotate the view vector by the horizontal angle around the vertical axis
    vec3 View(1.0f, 0.0f, 0.0f);
    View = rotate_y_deg(View, m_AngleH);
    View = normalise(View);

    // Rotate the view vector by the vertical angle around the horizontal axis
    Vector3f U = Yaxis.Cross(View);
    U.Normalize();
    View.Rotate(m_AngleV, U);

    m_target = View;
    m_target.Normalize();

    m_up = m_target.Cross(U);
    m_up.Normalize();
}



mat4 Camera::GetMatrix()
{
    mat4 CameraTransformation;
    CameraTransformation.InitCameraTransform(m_pos, m_target, m_up);

    return CameraTransformation;
}