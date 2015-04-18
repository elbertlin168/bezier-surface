#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdio>
#include <cstdlib>
#include <vector>


#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef BEZIER_H
#include "bezier.h"
#endif

#ifndef VECTOR_H
#include "vector.h"
#endif


//****************************************************
// Global Variables
//****************************************************

int windowWidth = 700;
int windowHeight = 700;
int windowID;
unsigned int mode = GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH;

float parameter;
bool objInput = false;
bool adaptive = false;
int wiremode = 0;           
int numPatches = 0;
int numdiv = 0;
std::vector<Vector**> patches; 
std::vector<Vector**> surfaces;
std::vector<Vector**> normals;

std::vector<Vector*> triangles;
std::vector<Vector*> trinormals;

float lpos[] = { 1000, 1000, 1000, 0 };
double xAngle = 0;
double yAngle = 0;
double zAngle = 180;
double xShift = 0.0;
double yShift = -1.0;
double zShift = -5.0;
double fovyFactor = 1.0;

//****************************************************
// OpenGL Functions
//****************************************************

void init(){
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    glLightfv(GL_LIGHT0, GL_POSITION, lpos);
    glLoadIdentity();
    if (adaptive) {
        for (int i = 0; i < numPatches; i++) {
            Bezier::adaptive_subdivide(patches.at(i), parameter, &triangles, &trinormals);
        }
    } else {
        numdiv = ceil(1.0f / parameter) + 1;
        for (int i = 0; i < numPatches; i++) {
            Bezier::uniform_subdivide(patches.at(i), parameter, &surfaces, &normals);
        }
    }
}

void display() {
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);    
    glLoadIdentity();
    glTranslatef(xShift, yShift, zShift);
    gluLookAt(0.0, 0.0, -5.0, 0, 0, -1, 0, 1, 0);
    glRotated(-90, 1, 0, 0);
    if (adaptive || objInput) {
        int numTriangles = triangles.size();
        glPushMatrix();
            glRotated(xAngle, 1, 0, 0);
            glRotated(zAngle, 0, 0, 1);
            glColor3f(1.0, 0.1, 0.1);
            glBegin(GL_TRIANGLES);
            for (int i = 0; i < numTriangles; i++) {
                Vector* triangle = triangles.at(i);
                Vector* trinormal = trinormals.at(i);
                glNormal3f(trinormal[0].x, trinormal[0].y, trinormal[0].z);
                glVertex3f(triangle[0].x, triangle[0].y, triangle[0].z);
                glNormal3f(trinormal[1].x, trinormal[1].y, trinormal[1].z);
                glVertex3f(triangle[1].x, triangle[1].y, triangle[1].z);
                glNormal3f(trinormal[2].x, trinormal[2].y, trinormal[2].z);
                glVertex3f(triangle[2].x, triangle[2].y, triangle[2].z);
            }
            glEnd();
        glPopMatrix();
    } else {
        for (int i = 0; i < numPatches; i++) {
            glPushMatrix();
                glRotated(xAngle, 1, 0, 0);
                glRotated(zAngle, 0, 0, 1);
                glColor3f(1.0, 0.1, 0.1);
                glBegin(GL_QUADS);
                Vector** surface = surfaces.at(i);
                Vector** normal = normals.at(i);
                for (int u = 0; u < numdiv - 1; u++) {
                    for (int v = 0; v < numdiv - 1; v++) {
                        glNormal3f(normal[u][v].x, normal[u][v].y, normal[u][v].z);
                        glVertex3f(surface[u][v].x, surface[u][v].y, surface[u][v].z);
                        glNormal3f(normal[u+1][v].x, normal[u+1][v].y, normal[u+1][v].z);
                        glVertex3f(surface[u+1][v].x, surface[u+1][v].y, surface[u+1][v].z);
                        glNormal3f(normal[u+1][v+1].x, normal[u+1][v+1].y, normal[u+1][v+1].z);
                        glVertex3f(surface[u+1][v+1].x, surface[u+1][v+1].y, surface[u+1][v+1].z);
                        glNormal3f(normal[u][v+1].x, normal[u][v+1].y, normal[u][v+1].z);
                        glVertex3f(surface[u][v+1].x, surface[u][v+1].y, surface[u][v+1].z);
                    }
                }
                glEnd();
            glPopMatrix();
        }
    }

    glFlush();
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float angle = fmin(50.0 * fovyFactor,  180.0f);
    angle = fmax(0.0, angle);
    gluPerspective(angle, (float) w / (float) h, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void keyboard(unsigned char key, int x, int y) {
    int window[4];
    switch (key) {
        case 's':
            GLint model;
            glGetIntegerv(GL_SHADE_MODEL, &model);
            if (model == GL_SMOOTH)
                glShadeModel(GL_FLAT);
            else
                glShadeModel(GL_SMOOTH);
            break;
        case 'w':
            if (wiremode != 1) {
                wiremode = 1;
                glDisable(GL_CULL_FACE);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            } else {
                wiremode = 0;
                glDisable(GL_CULL_FACE);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            break;
        /**
        case 'h':
            if (wiremode != 2) {
                wiremode = 2;
                glEnable(GL_CULL_FACE);
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            } else {
                wiremode = 0;
                glDisable(GL_CULL_FACE);
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            break;
        **/
        case '+':
            fovyFactor -= 0.01;
            glGetIntegerv(GL_VIEWPORT, window);
            glutReshapeWindow(window[2], window[3]);
            break;
        case '-':
            fovyFactor += 0.01;
            glGetIntegerv(GL_VIEWPORT, window);
            glutReshapeWindow(window[2], window[3]);
            break;

        case 27:
            glutDestroyWindow(windowID);
            break;
    }

    if (glutGetWindow())
        glutPostRedisplay();
}

void special(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:
            if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
                xShift -= 0.1;
            } else {
                zAngle -= 5;
            }
            break;
        case GLUT_KEY_RIGHT:
            if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
                xShift += 0.1;
            } else {
                zAngle += 5;
            }
            break;
        case GLUT_KEY_UP:
            if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
                yShift += 0.1;
            } else {
                xAngle += 5;
            }
            break;
        case GLUT_KEY_DOWN:
            if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
                yShift -= 0.1;
            } else {
                xAngle -= 5;
            }
            break;
    }
    glutPostRedisplay();
}

//****************************************************
// Input Parsing & Main
//****************************************************

bool isFloat(char* input) {
    if (!isdigit(input[0])) {
        if (input[0] == '+' || input[0] == '-') {
            if (!isdigit(input[1])) {
                if (input[1] == '.') {
                    if (isdigit(input[2]))
                        return true;
                }
                return false;
            }
            return true;
        } else if (input[0] == '.') {
            if (isdigit(input[1]))
                return true;
        } else if (input[0] == 'e' || input[0] == 'E') {
            if (!isdigit(input[1])) {
                if (input[1] == '+' || input[1] == '-') {
                    if (isdigit(input[2]))
                        return true;
                }
                return false;
            }
            return true;
        }
        return false;
    }
    return true;
}

void parse_bez_input(char* input) {
    int linecount = 0;
    int patchIndex = 0;
    int rowIndex = 0;
    bool setNum = false;
    Vector** patch;
    std::string line;
    std::ifstream file (input);

    if (file.is_open()) {
        std::cout << "Parsing .bez file..." << std::endl;

        while (std::getline(file, line)) {
            linecount++;
            char* tokens = new char[line.length() + 1];
            strcpy(tokens, line.c_str());
            tokens = strtok(tokens, " \n\t\r");

            if (tokens == NULL) {
                delete[] tokens;
                continue;
            }

            if (!setNum) {
                if (!isdigit(tokens[0])) {
                    std::cerr << "Line " << linecount <<
                            " was not formatted correctly." << std::endl;
                    file.close();
                    exit(EXIT_FAILURE);
                }
                numPatches = atoi(tokens);
                setNum = true;
                if (numPatches == 0)
                    std::cout << "Warning: 0 patches indicated." << std::endl;
                if (strtok(NULL, " \n\t\r") != NULL) {
                    std::cerr << "Line " << linecount <<
                            " contains extra values, which were ignored." <<
                            std::endl;
                }
                continue;
            }

            if (patchIndex < numPatches) {
                int i = 0;
                if (rowIndex == 0) {
                    patch = new Vector*[4];
                    for (int r = 0; r < 4; r++) {
                        patch[r] = new Vector[4];
                    }
                }

                while (tokens != NULL) {
                    if (i >= 12) {
                        std::cerr << "Line " << linecount <<
                                " contains extra values, which were ignored." <<
                                std::endl;
                        break;
                    }
                    if (!isFloat(tokens)) {
                        std::cerr << "Line " << linecount <<
                                " was not formatted correctly." << std::endl;
                        file.close();
                        exit(EXIT_FAILURE);
                    }
                    float x = atof(tokens);
                    tokens = strtok(NULL, " \n\t\r");
                    float y = atof(tokens);
                    tokens = strtok(NULL, " \n\t\r");
                    float z = atof(tokens);
                    tokens = strtok(NULL, " \n\t\r");
                    patch[rowIndex][i / 3] = Vector(x, y, z);
                    i+=3;
                }

                if (i < 12) {
                    std::cerr << "Line " << linecount <<
                            " does not contain enough values." << std::endl;
                    file.close();
                    exit(EXIT_FAILURE);
                }

                rowIndex++;
                if (rowIndex == 4) {
                    patches.push_back(patch);
                    patchIndex++;
                    rowIndex = 0;
                }
            }

            delete[] tokens;
        }

        file.close();
        if (patchIndex < numPatches) {
            std::cerr << "File does not contain the indicated number " <<
                    "of patches." << std::endl;
            exit(EXIT_FAILURE);
        }
        std::cout << "Finished parsing .bez file." << std::endl;
    } else {
        std::cerr << "Unable to open file: " << input << std::endl;
        exit(EXIT_FAILURE);
    }
}

void parse_obj_input(char* input) {
    int linecount = 0;
    std::vector<Vector> vertices;
    std::string line;
    std::ifstream file (input);

    // Create offset and provide "nonexistent" coordinate
    vertices.push_back(Vector(0, 0, 0));

    if (file.is_open()) {
        std::cout << "Parsing .obj file..." << std::endl;

        while (std::getline(file, line)) {
            linecount++;
            int i = 0;
            float values[3] = {0, 0, 0};
            char* tokens = new char[line.length() + 1];
            strcpy(tokens, line.c_str());
            tokens = strtok(tokens, " \n\t\r");

            if (tokens == NULL) {
                delete[] tokens;
                continue;
            } else if (strcmp(tokens, "v") == 0) {

                tokens = strtok(NULL, " \n\t\r");
                while (tokens != NULL) {
                    if (!isFloat(tokens)) {
                        std::cerr << "Line " << linecount <<
                                " was not formatted correctly." << std::endl;
                        file.close();
                        exit(EXIT_FAILURE);
                    }
                    if (i < 3)
                        values[i] = atof(tokens);
                    i++;
                    tokens = strtok(NULL, " \n\t\r");
                }

                if (i > 3) {
                    std::cerr << "Line " << linecount <<
                            " contains extra values, which were ignored." <<
                            std::endl;
                } else if (i < 3) {
                    std::cerr << "Line " << linecount <<
                            " does not contain enough values." << std::endl;
                    file.close();
                    exit(EXIT_FAILURE);
                }

                vertices.push_back(Vector(values[0], values[1], values[2]));

            } else if (strcmp(tokens, "f") == 0) {

                tokens = strtok(NULL, " \n\t\r");
                while (tokens != NULL) {
                    if (!isdigit(tokens[0])) {
                        std::cerr << "Line " << linecount <<
                                " was not formatted correctly." << std::endl;
                        file.close();
                        exit(EXIT_FAILURE);
                    }
                    if (i < 3)
                        values[i] = atof(tokens);
                    i++;
                    tokens = strtok(NULL, " \n\t\r");
                }

                if (i > 3) {
                    std::cerr << "Line " << linecount <<
                            " contains extra values, which were ignored." <<
                            std::endl;
                } else if (i < 3) {
                    std::cerr << "Line " << linecount <<
                            " does not contain enough values." << std::endl;
                    file.close();
                    exit(EXIT_FAILURE);
                }

                try {
                    Vector v1 = vertices.at((int) values[0]);
                    Vector v2 = vertices.at((int) values[1]);
                    Vector v3 = vertices.at((int) values[2]);
                    Vector U = v2 - v1;
                    Vector V = v3 - v1;
                    Vector normal = Vector::cross(U, V).normalize();
                    Vector *tri = new Vector[3];
                    Vector *norm = new Vector[3];
                    tri[0] = v1; tri[1] = v2; tri[2] = v3;
                    triangles.push_back(tri);
                    norm[0] = norm[1] = norm[2] = normal;
                    trinormals.push_back(norm);
                } catch (const std::out_of_range& e) {
                    std::cerr << "Line " << linecount <<
                            " does not contain valid parameters." << std::endl;
                    file.close();
                    exit(EXIT_FAILURE);
                }

            } else if (tokens[0] == '#') {
                delete[] tokens;
                continue;
            } else {
                std::cerr << "Command \"" << tokens << "\" of line " <<
                        linecount << " unrecognized." << std::endl;
                file.close();
                exit(EXIT_FAILURE);
            }

            delete[] tokens;
        }

        file.close();
        std::cout << "Finished parsing .obj file." << std::endl;
    } else {
        std::cerr << "Unable to open file: " << input << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Not enough input parameters." << std::endl;
        exit(EXIT_FAILURE);
    }

    char* ext = strpbrk(argv[1], ".");
    if (strcmp(ext, ".obj") == 0) {
        objInput = true;
        parse_obj_input(argv[1]);
    } else if (strcmp(ext, ".bez") == 0) {
        parse_bez_input(argv[1]);
    } else {
        std::cerr << "File format not recognized." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!objInput && argc < 3) {
        std::cerr << "Not enough input parameters." << std::endl;
        exit(EXIT_FAILURE);
    }

    if (!objInput)
        parameter = strtof(argv[2], NULL);
    
    if (argc > 3) {
        int argIndex = 3;
        while (argIndex < argc) {
            if (strcmp(argv[argIndex], "-a") == 0)
                adaptive = true;
            argIndex++;
        }
    }


    glutInit(&argc, argv);
    glutInitDisplayMode(mode);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(0, 0);
    windowID = glutCreateWindow(argv[0]);

    init();
    glutIdleFunc(glutPostRedisplay);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);

    

    glutMainLoop();

    return 0;
}
