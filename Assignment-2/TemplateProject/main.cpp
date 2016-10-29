/*
 Angela Wu
 CS6533 Prof Ivan Safrin
 Assignment 2
 10/28/16
 
 Render 3D scene with cubes
 At least 3 objects must be in a hierarchy
 */

#include "glsupport.h"
#include <GLUT/glut.h>
#include "matrix4.h"
#include "quat.h"
#include "geometrymaker.h"

GLuint program;
GLuint vertPositionVBO;
GLuint colorVBO;
GLuint normalVBO;

GLuint positionAttribute;
GLuint colorAttribute;
GLuint normalAttribute;

GLuint positionUniform;
GLuint modelviewMatrixUniformLocation;
GLuint projectionMatrixUniformLocation;
GLuint colorUniformLocation;
GLuint normalUniformLocation;
GLuint timeUniform;

struct VertexPN {
    Cvec3f p, n;
    
    VertexPN() {}
    
    VertexPN(float x, float y, float z, float nx, float ny, float nz) : p(x, y, z), n(nx, ny, nz) {}
    
    VertexPN& operator = (const GenericVertex& v) {
        p = v.pos;
        n = v.normal;
        return *this;
    }
};

struct Transform {
    Quat rotationX, rotationY, rotationZ;
    Quat rotation;
    Cvec3 scale;
    Cvec3 position;
    
    Transform() : scale(1.0f, 1.0f, 1.0f) {
    }
    
    Matrix4 createMatrix();
};

struct Geometry {
    GLuint vertexBO;
    GLuint indexBO;
    int numIndices;
    
    void Draw(GLuint positionAttribute, GLuint normalAttribute) {
        
        // bind buffer objects and draw
        glBindBuffer(GL_ARRAY_BUFFER, vertexBO);
        glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, p));
        glEnableVertexAttribArray(positionAttribute);
        
        glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, n));
        glEnableVertexAttribArray(normalAttribute);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBO);
        
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);
    }
};

struct Entity {
    Transform transform;
    Geometry geometry;
    Matrix4 modelViewMatrix;
    Entity* parent;
    
    void Draw(Matrix4 &eyeInverse, GLuint positionAttribute, GLuint normalAttribute, GLuint modelviewMatrixUniformLocation, GLuint normalMatrixUniformLocation)
    {
        Matrix4 parentMatrix;
        if (parent != nullptr) {
            parentMatrix = parent->modelViewMatrix;
        }
        
        Matrix4 modelMatrix = parentMatrix * modelViewMatrix;
        Matrix4 modelViewMatrix = eyeInverse * modelMatrix;
        
        Matrix4 normalMatrix = transpose(inv(modelViewMatrix));
        
        GLfloat glmatrix[16];
        modelViewMatrix.writeToColumnMajorMatrix(glmatrix);
        glUniformMatrix4fv(modelviewMatrixUniformLocation, 1, false, glmatrix);
        
        GLfloat glmatrixNormal[16];
        normalMatrix.writeToColumnMajorMatrix(glmatrixNormal);
        glUniformMatrix4fv(normalMatrixUniformLocation, 1, false, glmatrixNormal);
        
        geometry.Draw(positionAttribute, normalAttribute);
    }
};

float textureOffset = 0.0;

Entity cube1;
int iblen1;
int vbLen1;

Entity cube2;
int iblen2;
int vbLen2;

Entity cube3;
int iblen3;
int vbLen3;

GLuint color;

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
    glUniform1f(timeUniform, (float)timeSinceStart / 1000.0f);
    
    // Eye matrix
    Matrix4 eyeMatrix;
    eyeMatrix = eyeMatrix.makeTranslation(Cvec3(0.0, 0.0, 3.00));
    
    // Projection matrix
    Matrix4 projectionMatrix;
    projectionMatrix = projectionMatrix.makeProjection(100.0, 1.0, -0.1, -150.0);
    
    GLfloat glmatrixProjection[16];
    projectionMatrix.writeToColumnMajorMatrix(glmatrixProjection);
    glUniformMatrix4fv(projectionMatrixUniformLocation, 1, false, glmatrixProjection);
    
    Quat q1 = Quat::makeYRotation(0.0f*timeSinceStart*.001);
    Quat q2 = Quat::makeXRotation(40.0f*timeSinceStart*.001);
    
    Quat combined = q1 * q2;
    Matrix4 rotationMatrix = quatToMatrix(combined);
    Matrix4 modelViewMatrix = inv(eyeMatrix) * rotationMatrix;
    
    // Draw cubes
    glUniform3f(color, 0.0, 0.3, 1.0);
    cube1.Draw(modelViewMatrix, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalUniformLocation);
    
    glUniform3f(color, 0.2, 0.75, 1.0);
    cube2.Draw(modelViewMatrix, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalUniformLocation);
    
    glUniform3f(color, 0.0, 0.0, 0.0);
    cube3.Draw(modelViewMatrix, positionAttribute, normalAttribute, modelviewMatrixUniformLocation, normalUniformLocation);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    
    glutSwapBuffers();
}

void init() {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glReadBuffer(GL_BACK);
    
    program = glCreateProgram();
    readAndCompileShader(program, "vertex.glsl", "fragment.glsl");
    glUseProgram(program);
    
    positionAttribute = glGetAttribLocation(program, "position");
    colorAttribute = glGetAttribLocation(program, "color");
    normalAttribute = glGetAttribLocation(program, "normal");
    
    normalUniformLocation = glGetUniformLocation(program, "normalMatrix");
    colorUniformLocation = glGetUniformLocation(program, "uColor");
    modelviewMatrixUniformLocation = glGetUniformLocation(program, "modelViewMatrix");
    projectionMatrixUniformLocation = glGetUniformLocation(program, "projectionMatrix");
    
    // get parent cube
    cube3.parent = &cube2;
    cube2.parent = &cube1;
    
    // Cube 1
    getCubeVbIbLen(vbLen1, iblen1);
    cube1.geometry.indexBO = iblen1;
    cube1.geometry.vertexBO = vbLen1;
    cube1.geometry.numIndices = iblen1;
    
    std::vector<VertexPN> vtxS(vbLen1);
    std::vector<unsigned short> idxS(iblen1);
    
    makeCube(0.7, vtxS.begin(), idxS.begin());
    
    glGenBuffers(1, &cube1.geometry.vertexBO);
    glBindBuffer(GL_ARRAY_BUFFER, cube1.geometry.vertexBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vtxS.size(), vtxS.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &cube1.geometry.indexBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube1.geometry.indexBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idxS.size(), idxS.data(), GL_STATIC_DRAW);
    
    // Cube 2
    getCubeVbIbLen(vbLen2, iblen2);
    cube2.geometry.indexBO = iblen2;
    cube2.geometry.vertexBO = vbLen2;
    cube2.geometry.numIndices = iblen2;
    
    std::vector<VertexPN> vtxM(vbLen2);
    std::vector<unsigned short> idxM(iblen2);
    
    makeCube(1.6, vtxM.begin(), idxM.begin());
    
    glGenBuffers(1, &cube2.geometry.vertexBO);
    glBindBuffer(GL_ARRAY_BUFFER, cube2.geometry.vertexBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vtxM.size(), vtxM.data(), GL_STATIC_DRAW);
    glGenBuffers(1, &cube2.geometry.indexBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube2.geometry.indexBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idxM.size(), idxM.data(), GL_STATIC_DRAW);
    
    // Cube 3
    getCubeVbIbLen(vbLen3, iblen3);
    cube3.geometry.indexBO = iblen3;
    cube3.geometry.vertexBO = vbLen3;
    cube3.geometry.numIndices = iblen3;
    
    std::vector<VertexPN> vtxL(vbLen3);
    std::vector<unsigned short> idxL(iblen3);
    
    makeCube(2.3, vtxL.begin(), idxL.begin());
    
    glGenBuffers(1, &cube3.geometry.vertexBO);
    glBindBuffer(GL_ARRAY_BUFFER, cube3.geometry.vertexBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vtxL.size(), vtxL.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &cube3.geometry.indexBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube3.geometry.indexBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idxL.size(), idxL.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &normalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    GLfloat cubeNormals[] = {
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f,-1.0f,0.0f,
        0.0f,-1.0f,0.0f,
        0.0f,-1.0f,0.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        0.0f, 0.0f,-1.0f,
        -1.0f, 0.0f,0.0f,
        -1.0f, 0.0f,0.0f,
        -1.0f, 0.0f,0.0f,
        0.0f,-1.0f, 0.0f,
        0.0f,-1.0f, 0.0f,
        0.0f,-1.0f,0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f,0.0f, 1.0f,
        0.0f,0.0f, 1.0f,
        1.0f, 0.0f, 0.0f,
        1.0f,0.0f,0.0f,
        1.0f, 0.0f,0.0f,
        1.0f,0.0f,0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f,0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f,0.0f,
        0.0f, 1.0f,0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeNormals), cubeNormals, GL_STATIC_DRAW);
} 


void reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void idle(void) {
    glutPostRedisplay();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutCreateWindow("CS-6533 Assignment 2");
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    
    init();
    glutMainLoop();
    return 0;
}
