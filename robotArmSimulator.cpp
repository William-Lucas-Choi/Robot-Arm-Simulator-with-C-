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

// 各ロボットアームジョイントの角度
GLfloat joint1 = 0, joint2 = 0, joint3 = 0, joint4 = 0, joint5 = 0, joint6 = 0, joint7 = 0;
GLfloat targetJoint[7] = { 0, 0, 0, 0, 0, 0, 0 }; // 初期位置に移動する目標角度
GLfloat currentJoint[7] = { 0, 0, 0, 0, 0, 0, 0 }; // 現在の角度
bool isAnimating = false; // アニメーション中かどうか

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
        glutTimerFunc(16, reset_arm, 0); // 16msごとに呼び出し、スムーズなアニメーション効果を実現
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
        currentJoint[jointIndex] = slider->value(); // 角度をsliderの値に変更
        glutPostRedisplay(); // OpenGLの描き直し
    }
}

void resetButton_callback(Fl_Widget* widget, void* data) {
    isAnimating = true;
    glutTimerFunc(16, reset_arm, 0); // アームの角度リセット

    //glutPostRedisplay();
}

void init(void) {
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
}

void drawLink(float length, float width, float thick) {
    glBegin(GL_QUADS);

    // 前面 (Front face)
    glVertex3f(-width / 2.0f, 0.0f, thick / 2.0f);    // 左下
    glVertex3f(width / 2.0f, 0.0f, thick / 2.0f);     // 右下
    glVertex3f(width / 2.0f, length, thick / 2.0f);   // 右上
    glVertex3f(-width / 2.0f, length, thick / 2.0f);  // 左上

    // 背面 (Back face)
    glVertex3f(-width / 2.0f, 0.0f, -thick / 2.0f);    // 左下
    glVertex3f(width / 2.0f, 0.0f, -thick / 2.0f);     // 右下
    glVertex3f(width / 2.0f, length, -thick / 2.0f);   // 右上
    glVertex3f(-width / 2.0f, length, -thick / 2.0f);  // 左上

    // 左側面 (Left face)
    glVertex3f(-width / 2.0f, 0.0f, thick / 2.0f);       // 左下
    glVertex3f(-width / 2.0f, 0.0f, -thick / 2.0f);    // 左下
    glVertex3f(-width / 2.0f, length, -thick / 2.0f);  // 左上
    glVertex3f(-width / 2.0f, length, thick / 2.0f);     // 左上

    // 右側面 (Right face)
    glVertex3f(width / 2.0f, 0.0f, thick / 2.0f);       // 右下
    glVertex3f(width / 2.0f, 0.0f, -thick / 2.0f);    // 右下
    glVertex3f(width / 2.0f, length, -thick / 2.0f);  // 右上
    glVertex3f(width / 2.0f, length, thick / 2.0f);     // 右上

    // 上面 (Top face)
    glVertex3f(-width / 2.0f, length, thick / 2.0f);      // 左前
    glVertex3f(width / 2.0f, length, thick / 2.0f);       // 右前
    glVertex3f(width / 2.0f, length, -thick / 2.0f);    // 右後
    glVertex3f(-width / 2.0f, length, -thick / 2.0f);   // 左後

    // 下面 (Bottom face)
    glVertex3f(-width / 2.0f, 0.0f, thick / 2.0f);      // 左前
    glVertex3f(width / 2.0f, 0.0f, thick / 2.0f);       // 右前
    glVertex3f(width / 2.0f, 0.0f, -thick / 2.0f);    // 右後
    glVertex3f(-width / 2.0f, 0.0f, -thick / 2.0f);   // 左後
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

            // 現在のスライス
            float x = radius * cos(theta) * cos(phi);
            float y = radius * sin(theta) * cos(phi);
            float z = radius * sin(phi);
            glColor3f(0.7f, 0.0f, 0.0f); // 色変更
            glVertex3f(x, y, z);

            // 次のスライス
            x = radius * cos(theta) * cos(nextPhi);
            y = radius * sin(theta) * cos(nextPhi);
            z = radius * sin(nextPhi);
            glColor3f(0.7f, 0.0f, 0.0f); // 色変更
            glVertex3f(x, y, z);
        }
        glEnd();
    }
}

void drawRobotArm() {
    float linkLength = 0.3f; // 各リンクの長さ
    float linkwidth = 0.6f; // 各リンクの幅
    float linkthick = 0.6f; // 各リンクの厚さ
    float radius = 0.15f; // 各ジョイントの半径

    glPushMatrix();
    glTranslatef(-2.0f, -4 * linkLength, 0.0f); // 最初のリンクの開始点に移動

    // リンクとジョイントを作成
    for (int i = 0; i < 2; ++i) {
        glTranslatef(0.0f, linkLength / 3, 0.0f); // リンクの開始点に移動
        glRotatef(currentJoint[i], 0.0f, 1.0f, 0.0f); // ジョイントの角度分回転

        if (i != 0)
        {
            glColor3f(0.7f, 0.7f, 0.7f); // 色変更
            linkLength = 0.4f;
            drawLink(linkLength, linkwidth, linkthick); // リンクを作成
            glTranslatef(0.0f, linkLength, 0.0f); // ジョイントの位置に移動
            drawJoint(radius);
        }
        else {
            glColor3f(0.3f, 0.3f, 0.3f); // 色変更
            linkLength = 0.2f;
            drawLink(linkLength, linkwidth, linkthick); // リンクを作成
            glTranslatef(0.0f, linkLength / 2, 0.0f);
        }// ジョイントを作成
    }
    glTranslatef(0.0f, linkLength / 3, 0.0f); // リンクの開始点に移動
    glColor3f(0.4f, 0.4f, 0.4f); // 色変更

    linkLength = 0.6f; // リンクの長さを変更
    linkwidth = 0.5f; // リンクの幅を変更
    linkthick = 0.4f; // リンクの厚さを変更
    radius = 0.15f; // ジョイントの半径を変更

    for (int i = 2; i < 7; ++i) {
        // 各リンクとジョイントの作成
        switch (i) {
        case 3:
            linkLength = 0.6f; // リンクの長さを変更
            linkwidth = 0.5f; // リンクの幅を変更
            linkthick = 0.4f; // リンクの厚さを変更
            radius = 0.13f; // ジョイントの半径を変更
            glColor3f(0.9f, 0.9f, 0.9f); // 色変更
            break;
        case 4:
            linkLength = 0.7f; // リンクの長さを変更
            linkwidth = 0.4f; // リンクの幅を変更
            linkthick = 0.4f; // リンクの厚さを変更
            radius = 0.08f; // ジョイントの半径を変更
            glColor3f(0.4f, 0.4f, 0.4f); // 色変更
            break;
        case 5:
            linkLength = 0.2f; // リンクの長さを変更
            linkwidth = 0.35f; // リンクの幅を変更
            linkthick = 0.35f; // リンクの厚さを変更
            radius = 0.1f; // ジョイントの半径を変更
            glColor3f(0.0f, 0.5f, 0.5f); // 色変更
            break;
        case 6:
            linkLength = 0.8f; // リンクの長さを変更
            linkwidth = 0.2f; // リンクの幅を変更
            linkthick = 0.2f; // リンクの厚さを変更
            radius = 0.03f; // ジョイントの半径を変更
            glColor3f(1.0f, 1.0f, 1.0f); // 色変更
            break;
        }

        if (i != 5) glRotatef(currentJoint[i], 0.0f, 0.0f, 1.0f); // ジョイントの角度分回転
        else glRotatef(currentJoint[i], 0.0f, 1.0f, 0.0f); // ジョイントの角度分回転
        drawLink(linkLength, linkwidth, linkthick); // リンクを作成
        glTranslatef(0.0f, linkLength, 0.0f); // ジョイントの位置に移動
        if (i != 6) drawJoint(radius); // ジョイントを作成
        glTranslatef(0.0f, 0.75 * radius, 0.0f); // リンクの開始点に移動
    }
    glPopMatrix();

}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix();
    // アーム作成
    drawRobotArm();
    glPopMatrix();
    glutSwapBuffers();
    for (int i = 1; i < 7; ++i) {   //リアルタイムでスライダーの位置表示
        jointSliders[i]->value(currentJoint[i]); // sliderの値をアップデート
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
    // FLTK window・widget 作成
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

    // OpenGL 初期化
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
