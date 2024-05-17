#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Gl_Window.H>
#include <cmath>
#include <cstdio>
#include <string>

#include <GL/glew.h>
#include <FL/gl.h>
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// �e���{�b�g�A�[���W���C���g�̊p�x
GLfloat joint1 = 0, joint2 = 0, joint3 = 0, joint4 = 0, joint5 = 0, joint6 = 0, joint7 = 0;
GLfloat targetJoint[7] = { 0, 0, 0, 0, 0, 0, 0 }; // �����ʒu�Ɉړ�����ڕW�p�x
GLfloat currentJoint[7] = { 0, 0, 0, 0, 0, 0, 0 }; // ���݂̊p�x
bool isAnimating = false; // �A�j���[�V���������ǂ���

Fl_Window* mainWindow;
Fl_Slider* jointSliders[7];
Fl_Button* resetButton;

void updateRobotArm() {
    glutPostRedisplay();
}

void reset_arm(int value) {
    for (int i = 1; i < 7; ++i) {
        if (currentJoint[i] != targetJoint[i]) {
            if (fabs(currentJoint[i] - targetJoint[i]) < 1.0f) {
                currentJoint[i] = targetJoint[i];
            }
            else if (currentJoint[i] < targetJoint[i]) {
                currentJoint[i] += 1.0f;
            }
            else {
                currentJoint[i] -= 1.0f;
            }
        }
    }

    bool allJointsReset = true;
    for (int i = 1; i < 7; ++i) {
        if (currentJoint[i] != targetJoint[i]) {
            allJointsReset = false;
            break;
        }
    }

    if (!allJointsReset) {
        glutTimerFunc(16, reset_arm, 0); // 16ms���ƂɌĂяo���A�X���[�Y�ȃA�j���[�V�������ʂ�����
    }
    else {
        isAnimating = false;
    }

    updateRobotArm();
}

void slider_callback(Fl_Widget* widget, void* data) {
    /*Fl_Slider* slider = (Fl_Slider*)widget;
    int index = reinterpret_cast<int>(data);
    currentJoint[index] = slider->value();
    updateRobotArm();*/
    int jointIndex = reinterpret_cast<int>(data);
    Fl_Slider* slider = dynamic_cast<Fl_Slider*>(widget);
    if (slider) {
        currentJoint[jointIndex] = slider->value(); // �p�x��slider�̒l�ɕύX
        glutPostRedisplay(); // OpenGL�̕`������
    }
}

void resetButton_callback(Fl_Widget* widget, void* data) {
    isAnimating = true;
    glutTimerFunc(16, reset_arm, 0); // �A�[���̊p�x���Z�b�g

    //glutPostRedisplay();
}

void init(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
}

void drawLink(float length, float width, float thick) {
    glBegin(GL_QUADS);

    // �O�� (Front face)
    glVertex3f(-width / 2.0f, 0.0f, thick / 2.0f);    // ����
    glVertex3f(width / 2.0f, 0.0f, thick / 2.0f);     // �E��
    glVertex3f(width / 2.0f, length, thick / 2.0f);   // �E��
    glVertex3f(-width / 2.0f, length, thick / 2.0f);  // ����

    // �w�� (Back face)
    glVertex3f(-width / 2.0f, 0.0f, -thick / 2.0f);    // ����
    glVertex3f(width / 2.0f, 0.0f, -thick / 2.0f);     // �E��
    glVertex3f(width / 2.0f, length, -thick / 2.0f);   // �E��
    glVertex3f(-width / 2.0f, length, -thick / 2.0f);  // ����

    // ������ (Left face)
    glVertex3f(-width / 2.0f, 0.0f, thick / 2.0f);       // ����
    glVertex3f(-width / 2.0f, 0.0f, -thick / 2.0f);    // ����
    glVertex3f(-width / 2.0f, length, -thick / 2.0f);  // ����
    glVertex3f(-width / 2.0f, length, thick / 2.0f);     // ����

    // �E���� (Right face)
    glVertex3f(width / 2.0f, 0.0f, thick / 2.0f);       // �E��
    glVertex3f(width / 2.0f, 0.0f, -thick / 2.0f);    // �E��
    glVertex3f(width / 2.0f, length, -thick / 2.0f);  // �E��
    glVertex3f(width / 2.0f, length, thick / 2.0f);     // �E��

    // ��� (Top face)
    glVertex3f(-width / 2.0f, length, thick / 2.0f);      // ���O
    glVertex3f(width / 2.0f, length, thick / 2.0f);       // �E�O
    glVertex3f(width / 2.0f, length, -thick / 2.0f);    // �E��
    glVertex3f(-width / 2.0f, length, -thick / 2.0f);   // ����

    // ���� (Bottom face)
    glVertex3f(-width / 2.0f, 0.0f, thick / 2.0f);      // ���O
    glVertex3f(width / 2.0f, 0.0f, thick / 2.0f);       // �E�O
    glVertex3f(width / 2.0f, 0.0f, -thick / 2.0f);    // �E��
    glVertex3f(-width / 2.0f, 0.0f, -thick / 2.0f);   // ����
    glEnd();
}

void drawJoint(float radius) {
    const int slices = 50;
    const float angleStep = M_PI / slices;

    for (int j = 0; j < slices; ++j) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= slices; ++i) {
            float theta = i * angleStep;
            float phi = -M_PI / 2.0f + j * angleStep;
            float nextPhi = -M_PI / 2.0f + (j + 1) * angleStep;

            // ���݂̃X���C�X
            float x = radius * cos(theta) * cos(phi);
            float y = radius * sin(theta) * cos(phi);
            float z = radius * sin(phi);
            glColor3f(0.7f, 0.0f, 0.0f); // �F�ύX
            glVertex3f(x, y, z);

            // ���̃X���C�X
            x = radius * cos(theta) * cos(nextPhi);
            y = radius * sin(theta) * cos(nextPhi);
            z = radius * sin(nextPhi);
            glColor3f(0.7f, 0.0f, 0.0f); // �F�ύX
            glVertex3f(x, y, z);
        }
        glEnd();
    }
}

void drawRobotArm() {
    float linkLength = 0.3f; // �e�����N�̒���
    float linkwidth = 0.6f; // �e�����N�̕�
    float linkthick = 0.6f; // �e�����N�̌���
    float radius = 0.15f; // �e�W���C���g�̔��a

    glPushMatrix();
    glTranslatef(-2.0f, -4 * linkLength, 0.0f); // �ŏ��̃����N�̊J�n�_�Ɉړ�

    // �����N�ƃW���C���g���쐬
    for (int i = 0; i < 2; ++i) {
        glTranslatef(0.0f, linkLength / 3, 0.0f); // �����N�̊J�n�_�Ɉړ�
        glRotatef(currentJoint[i], 0.0f, 1.0f, 0.0f); // �W���C���g�̊p�x����]

        if (i != 0)
        {
            glColor3f(0.7f, 0.7f, 0.7f); // �F�ύX
            linkLength = 0.4f;
            drawLink(linkLength, linkwidth, linkthick); // �����N���쐬
            glTranslatef(0.0f, linkLength, 0.0f); // �W���C���g�̈ʒu�Ɉړ�
            drawJoint(radius);
        }
        else {
            glColor3f(0.3f, 0.3f, 0.3f); // �F�ύX
            linkLength = 0.2f;
            drawLink(linkLength, linkwidth, linkthick); // �����N���쐬
            glTranslatef(0.0f, linkLength / 2, 0.0f);
        }// �W���C���g���쐬
    }
    glTranslatef(0.0f, linkLength / 3, 0.0f); // �����N�̊J�n�_�Ɉړ�
    glColor3f(0.4f, 0.4f, 0.4f); // �F�ύX

    linkLength = 0.6f; // �����N�̒�����ύX
    linkwidth = 0.5f; // �����N�̕���ύX
    linkthick = 0.4f; // �����N�̌�����ύX
    radius = 0.15f; // �W���C���g�̔��a��ύX

    for (int i = 2; i < 7; ++i) {
        // �e�����N�ƃW���C���g�̍쐬
        switch (i) {
        case 3:
            linkLength = 0.6f; // �����N�̒�����ύX
            linkwidth = 0.5f; // �����N�̕���ύX
            linkthick = 0.4f; // �����N�̌�����ύX
            radius = 0.13f; // �W���C���g�̔��a��ύX
            glColor3f(0.9f, 0.9f, 0.9f); // �F�ύX
            break;
        case 4:
            linkLength = 0.7f; // �����N�̒�����ύX
            linkwidth = 0.4f; // �����N�̕���ύX
            linkthick = 0.4f; // �����N�̌�����ύX
            radius = 0.08f; // �W���C���g�̔��a��ύX
            glColor3f(0.4f, 0.4f, 0.4f); // �F�ύX
            break;
        case 5:
            linkLength = 0.2f; // �����N�̒�����ύX
            linkwidth = 0.35f; // �����N�̕���ύX
            linkthick = 0.35f; // �����N�̌�����ύX
            radius = 0.1f; // �W���C���g�̔��a��ύX
            glColor3f(0.0f, 0.5f, 0.5f); // �F�ύX
            break;
        case 6:
            linkLength = 0.8f; // �����N�̒�����ύX
            linkwidth = 0.2f; // �����N�̕���ύX
            linkthick = 0.2f; // �����N�̌�����ύX
            radius = 0.03f; // �W���C���g�̔��a��ύX
            glColor3f(1.0f, 1.0f, 1.0f); // �F�ύX
            break;
        }

        if (i != 5) glRotatef(currentJoint[i], 0.0f, 0.0f, 1.0f); // �W���C���g�̊p�x����]
        else glRotatef(currentJoint[i], 0.0f, 1.0f, 0.0f); // �W���C���g�̊p�x����]
        drawLink(linkLength, linkwidth, linkthick); // �����N���쐬
        glTranslatef(0.0f, linkLength, 0.0f); // �W���C���g�̈ʒu�Ɉړ�
        if (i != 6) drawJoint(radius); // �W���C���g���쐬
        glTranslatef(0.0f, 0.75 * radius, 0.0f); // �����N�̊J�n�_�Ɉړ�
    }
    glPopMatrix();

}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix();
    // �A�[���쐬
    drawRobotArm();
    glPopMatrix();
    glutSwapBuffers();
    for (int i = 1; i < 7; ++i) {   //���A���^�C���ŃX���C�_�[�̈ʒu�\��
        jointSliders[i]->value(currentJoint[i]); // slider�̒l���A�b�v�f�[�g
    }
}

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -5.0);
}

int main(int argc, char** argv) {
    // FLTK window�Ewidget �쐬
    mainWindow = new Fl_Window(600, 600, "Robot Arm Control");
    for (int i = 1; i < 7; ++i) {
        jointSliders[i] = new Fl_Slider(20, 20 + i * 40, 560, 20);
        jointSliders[i]->type(FL_HORIZONTAL);
        switch (i) {
            /*case 0: break;*/
        case 1: jointSliders[i]->range(-110, 80);
            break;
        case 5: jointSliders[i]->range(-180, 180);
            break;
        default: jointSliders[i]->range(100, -100);
        }
        jointSliders[i]->step(1);
        jointSliders[i]->value(currentJoint[i]);
        jointSliders[i]->callback(slider_callback, reinterpret_cast<void*>(i));
    }
    resetButton = new Fl_Button(20, 300, 100, 40, "Reset");
    resetButton->callback(resetButton_callback);

    mainWindow->end();
    mainWindow->show(argc, argv);

    // OpenGL ������
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMainLoop();

    return 0;
}
