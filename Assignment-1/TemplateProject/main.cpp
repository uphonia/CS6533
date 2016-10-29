#include <GLUT/glut.h>
#include "glsupport.h"

GLuint program;
GLuint vertPositionVBO;
GLuint vertColorVBO;
GLuint positionAttribute;
GLuint colorAttribute;
GLuint texCoordAttribute;
GLuint vertTexCoordVBO;
GLuint timeUniform;
GLuint positionUniform;
GLuint pogoTexture;

float textureOffset = 0.0;

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    timeUniform = glGetUniformLocation(program, "time");
    positionUniform = glGetUniformLocation(program, "modelPosition");
    
    // Rendering vertex buffer
    //glUseProgram(program);
    glBindBuffer(GL_ARRAY_BUFFER, vertPositionVBO);
    glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionAttribute);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertTexCoordVBO);
    glVertexAttribPointer(texCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, pogoTexture);
    
    glUniform2f(positionUniform, -0.5, 0.0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glUniform2f(positionUniform, 0.5, 0.0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(positionAttribute);
    glDisableVertexAttribArray(colorAttribute);
    glDisableVertexAttribArray(texCoordAttribute);
    
    glutSwapBuffers();
}

void init() {
    program = glCreateProgram();
    pogoTexture = loadGLTexture("pogo.png");
    
    // Load shader program
    readAndCompileShader(program, "vertex.glsl", "fragment.glsl");
    
    // Get attribute location in program
    glUseProgram(program);
    positionAttribute = glGetAttribLocation(program, "position");
    colorAttribute = glGetAttribLocation(program, "color");
    texCoordAttribute = glGetAttribLocation(program, "texCoord");
    
    // Create vertex buffer object (VBO)
    glGenBuffers(1, &vertPositionVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertPositionVBO);
    
    GLfloat sqVerts[12] = {
        -0.5f, -0.5f,
        0.5f, 0.5f,
        0.5f, -0.5f,
        
        -0.5f, -0.5f,
        -0.5f, 0.5f,
        0.5f, 0.5f
    };
    
    glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), sqVerts, GL_STATIC_DRAW);
    
    glGenBuffers(1, &vertColorVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertColorVBO);
    
    GLfloat sqColors[24] = {
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 1.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, 24*sizeof(GLfloat), sqColors, GL_STATIC_DRAW);
    
    glGenBuffers(1, &vertTexCoordVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertTexCoordVBO);
    
    GLfloat sqTexCoords[12] = {
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), sqTexCoords, GL_STATIC_DRAW);
    
    // Blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void idle(void) {
    glutPostRedisplay();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(640, 640);
    glutCreateWindow("CS-6533");
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    
    init();
    glutMainLoop();
    return 0;
}
