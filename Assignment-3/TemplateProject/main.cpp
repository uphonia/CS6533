/*
 Angela Wu
 CS6533 Prof Ivan Safrin
 Assignment 3
 11/26/16
 
 Render scene with objects or models
 At least 3 positional lights
 */

#define TINYOBJLOADER_IMPLEMENTATION

#include "glsupport.h"
#include <GLUT/glut.h>
#include "matrix4.h"
#include "quat.h"
#include "geometrymaker.h"
#include "tiny_obj_loader.h"

struct Entity;

GLuint program;

GLuint positionAttribute, texCoordAttribute;
GLuint normalAttribute, binormalAttribute, tangentAttribute;
GLuint modelViewMatrixLoc, projectionMatrixLoc, normalMatrixLoc;

GLuint diffuseTexUniformLoc, specularTexUniformLoc, normalTextureLoc;

GLuint lightPositionUniformLocation0, lightColorUniform0, specularLightUniform0;
GLuint lightPositionUniformLocation1, lightColorUniform1, specularLightUniform1;
GLuint lightPositionUniformLocation2, lightColorUniform2, specularLightUniform2;

struct VertexPNTBTG {
    Cvec3f p, n, b, tg;
    Cvec2f t;
    
    VertexPNTBTG() {};
    VertexPNTBTG(float x, float y, float z, float nx, float ny, float nz) : p(x, y, z), n(nx, ny, nz) {}
    
    VertexPNTBTG& operator = (const GenericVertex& v) {
        p = v.pos;
        n = v.normal;
        t = v.tex;
        b = v.binormal;
        tg = v.tangent;
        return *this;
    }
};

struct Transform {
    Cvec3 translation;
    Quat rotation;
    Cvec3 scale;
    
    Transform() : scale(1.0, 1.0, 1.0) {}
    
    Matrix4 createMatrix() {
        Matrix4 transformMatrix;
        transformMatrix = transformMatrix.makeTranslation(translation) * quatToMatrix(rotation) * transformMatrix.makeScale(scale);
        return transformMatrix;
    }
};

struct Geometry {
    GLuint vertexVBO;
    GLuint indexBO;
    int numIndices;
    
    void Draw(GLuint positionAttribute, GLuint texCoordAttribute, GLuint normalAttribute, GLuint binormalAttribute,
              GLuint tangentAttribute)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
        glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPNTBTG), (void*)offsetof(VertexPNTBTG, p));
        glEnableVertexAttribArray(positionAttribute);
        
        glVertexAttribPointer(texCoordAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPNTBTG), (void*)offsetof(VertexPNTBTG, t));
        glEnableVertexAttribArray(texCoordAttribute);
        
        glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPNTBTG), (void*)offsetof(VertexPNTBTG, n));
        glEnableVertexAttribArray(normalAttribute);
        
        glVertexAttribPointer(binormalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPNTBTG), (void*)offsetof(VertexPNTBTG, b));
        glEnableVertexAttribArray(binormalAttribute);
        
        glVertexAttribPointer(tangentAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPNTBTG), (void*)offsetof(VertexPNTBTG, tg));
        glEnableVertexAttribArray(tangentAttribute);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBO);
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);
        
        glDisableVertexAttribArray(positionAttribute);
        glDisableVertexAttribArray(texCoordAttribute);
        glDisableVertexAttribArray(normalAttribute);
        glDisableVertexAttribArray(binormalAttribute);
        glDisableVertexAttribArray(tangentAttribute);
    }
};

struct Entity {
    Transform transform;
    Geometry geometry;
    Entity *parent;
    
    std::vector<VertexPNTBTG> meshVertices;
    std::vector<unsigned short> meshIndices;
    
    GLuint diffuseTexture, normalTexture, specularTexture;
    
    Matrix4 getModelViewMatrix() {
        Matrix4 modelViewMatrix = transform.createMatrix();
        if (parent != nullptr) {
            return parent->getModelViewMatrix() * modelViewMatrix;
        }
        return modelViewMatrix;
    }
    
    void Draw(Matrix4 &eyeInverse, GLuint positionAttribute, GLuint texCoordAttribute, GLuint normalAttribute, GLuint binormalAttribute, GLuint tangentAttribute, GLuint modelViewMatrixLoc, GLuint normalMatrixloc) {
        
        Matrix4 parModel;
        if (parent != nullptr) {
            parModel = parent->getModelViewMatrix();
        }
        
        Matrix4 modelMatrix = parModel * getModelViewMatrix();
        Matrix4 modelViewMatrix = eyeInverse * modelMatrix;
        Matrix4 normMatrix = normalMatrix(modelViewMatrix);
        
        GLfloat glmatrix[16];
        modelViewMatrix.writeToColumnMajorMatrix(glmatrix);
        glUniformMatrix4fv(modelViewMatrixLoc, 1, false, glmatrix);
        
        GLfloat glmatrixNormal[16];
        normMatrix.writeToColumnMajorMatrix(glmatrixNormal);
        glUniformMatrix4fv(normalMatrixLoc, 1, false, glmatrixNormal);
        
        geometry.Draw(positionAttribute, texCoordAttribute, normalAttribute, binormalAttribute, tangentAttribute);
    }
};

Entity monk1, monk2;

void calculateFaceTangent(const Cvec3f &v1, const Cvec3f &v2, const Cvec3f &v3, const Cvec2f &texCoord1,
                          const Cvec2f &texCoord2, const Cvec2f &texCoord3, Cvec3f &tangent, Cvec3f &binormal)
{
    Cvec3f side0 = v1 - v2;
    Cvec3f side1 = v3 - v1;
    Cvec3f normal = cross(side1, side0);
    
    normalize(normal);
    
    float deltaV0 = texCoord1[1] - texCoord2[1];
    float deltaV1 = texCoord3[1] - texCoord1[1];
    tangent = side0 * deltaV1 - side1 * deltaV0;
    
    normalize(tangent);
    
    float deltaU0 = texCoord1[0] - texCoord2[0];
    float deltaU1 = texCoord3[0] - texCoord1[0];
    binormal = side0 * deltaU1 - side1 * deltaU0;
    
    normalize(binormal);
    
    Cvec3f tangentCross = cross(tangent, binormal);
    
    if (dot(tangentCross, normal) < 0.0f) {
        tangent = tangent * -1;
    }
}

void loadObjFile(const std::string &fileName, std::vector<VertexPNTBTG> &outVertices, std::vector<unsigned short> &outIndices){
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fileName.c_str(), NULL, true);
    
    if (ret) {
        for (int i = 0; i < shapes.size(); i++) {
            for (int j = 0; j < shapes[i].mesh.indices.size(); j++) {
                unsigned int vertexOffest = shapes[i].mesh.indices[j].vertex_index * 3;
                unsigned int normalOffest = shapes[i].mesh.indices[j].normal_index * 3;
                unsigned int texOffset = shapes[i].mesh.indices[j].texcoord_index * 2;
                VertexPNTBTG v;
                v.p[0] = attrib.vertices[vertexOffest];
                v.p[1] = attrib.vertices[vertexOffest + 1];
                v.p[2] = attrib.vertices[vertexOffest + 2];
                v.n[0] = attrib.normals[normalOffest];
                v.n[1] = attrib.normals[normalOffest + 1];
                v.n[2] = attrib.normals[normalOffest + 2];
                v.t[0] = attrib.texcoords[texOffset];
                v.t[1] = 1.0 - attrib.texcoords[texOffset + 1];
                outVertices.push_back(v);
                outIndices.push_back(outVertices.size() - 1);
            }
        }
        
        for (int i = 0; i < outVertices.size(); i += 3) {
            Cvec3f tangent;
            Cvec3f binormal;
            
            calculateFaceTangent(outVertices[i].p, outVertices[i + 1].p, outVertices[i + 2].p, outVertices[i].t,
                                 outVertices[i + 1].t, outVertices[i + 2].t, tangent, binormal);
            
            outVertices[i].tg = tangent;
            outVertices[i + 1].tg = tangent;
            outVertices[i + 2].tg = tangent;
            
            outVertices[i].b = binormal;
            outVertices[i + 1].b = binormal;
            outVertices[i + 2].b = binormal;
        }
    }
    else {
        std::cout << err << std::endl;
        assert(false);
    }
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    float timeElapsed = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    float angle = timeElapsed * 15.0;
    
    Matrix4 eyeMatrix;
    eyeMatrix = eyeMatrix.makeTranslation(Cvec3(-1.0, 12.0, 25.0));
    eyeMatrix = eyeMatrix * eyeMatrix.makeXRotation(-15.0);
    Matrix4 modelMatrix = inv(eyeMatrix);
    
    Cvec4 lightPosition0 = Cvec4(-2.0, 10.0, 5.0, 1.0);
    lightPosition0 = inv(eyeMatrix) * lightPosition0;
    
    glUniform3f(lightPositionUniformLocation0, lightPosition0[0], lightPosition0[1], lightPosition0[2]);
    glUniform3f(lightColorUniform0, 1.0, 0.3, 10.0);
    glUniform3f(specularLightUniform0, 0.0, 1.0, 1.0);
    
    Cvec4 lightPosition1 = Cvec4(5.0, 15.0, 15.0, 1.0);
    lightPosition1 = inv(eyeMatrix) * lightPosition1;
    
    glUniform3f(lightPositionUniformLocation1, lightPosition1[0], lightPosition1[1], lightPosition1[2]);
    glUniform3f(lightColorUniform1, 0.0, 0.3, 1.0);
    glUniform3f(specularLightUniform1, 0.75, 0.9, 1.0);
    
    Cvec4 lightPosition2 = Cvec4(-2.0, 5.0, 0.0, 1.0);
    lightPosition2 = inv(eyeMatrix) * lightPosition2;
    
    glUniform3f(lightPositionUniformLocation2, lightPosition2[0], lightPosition2[1], lightPosition2[2]);
    glUniform3f(lightColorUniform2, 0.23, 0.74, 0.5);
    glUniform3f(specularLightUniform2, 0.5, 0.5, 1.0);
    
    Matrix4 projectionMatrix;
    projectionMatrix = projectionMatrix.makeProjection(45.0, 1.0, -0.1, -100.0);
    
    GLfloat glmatrixProjection[16];
    projectionMatrix.writeToColumnMajorMatrix(glmatrixProjection);
    glUniformMatrix4fv(projectionMatrixLoc, 1, false, glmatrixProjection);
    
    monk1.transform.rotation = Quat::makeYRotation(angle);
    monk1.Draw(modelMatrix, positionAttribute, texCoordAttribute, normalAttribute, binormalAttribute, tangentAttribute, modelViewMatrixLoc, normalMatrixLoc);
    
    monk2.transform.rotation = Quat::makeYRotation(90.0);
    monk2.transform.translation = Cvec3(-5.0, 0.0, -5.0);
    monk2.Draw(modelMatrix, positionAttribute, texCoordAttribute, normalAttribute, binormalAttribute, tangentAttribute, modelViewMatrixLoc, normalMatrixLoc);
    
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void idle(void) {
    glutPostRedisplay();
}

void init() {
    glClearDepth(0.0f);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    glReadBuffer(GL_BACK);
    glEnable(GL_LIGHTING);
    
    program = glCreateProgram();
    readAndCompileShader(program, "vertex.glsl", "fragment.glsl");
    glUseProgram(program);
    
    positionAttribute = glGetAttribLocation(program, "position");
    texCoordAttribute = glGetAttribLocation(program, "texCoord");
    
    normalAttribute = glGetAttribLocation(program, "normal");
    binormalAttribute = glGetAttribLocation(program, "binormal");
    tangentAttribute = glGetAttribLocation(program, "tangent");
    
    modelViewMatrixLoc = glGetUniformLocation(program, "modelViewMatrix");
    projectionMatrixLoc = glGetUniformLocation(program, "projectionMatrix");
    normalMatrixLoc = glGetUniformLocation(program, "normalMatrix");
    
    diffuseTexUniformLoc = glGetUniformLocation(program, "diffuseTexture");
    specularTexUniformLoc = glGetUniformLocation(program, "specularTexture");
    normalTextureLoc = glGetUniformLocation(program, "normalTexture");
    
    lightPositionUniformLocation0 = glGetUniformLocation(program, "lights[0].lightPosition");
    lightColorUniform0 = glGetUniformLocation(program, "lights[0].lightColor");
    specularLightUniform0 = glGetUniformLocation(program, "lights[0].specularLightColor");
    
    lightPositionUniformLocation1 = glGetUniformLocation(program, "lights[1].lightPosition");
    lightColorUniform1 = glGetUniformLocation(program, "lights[1].lightColor");
    specularLightUniform1 = glGetUniformLocation(program, "lights[1].specularLightColor");
    
    lightPositionUniformLocation2 = glGetUniformLocation(program, "lights[2].lightPosition");
    lightColorUniform2 = glGetUniformLocation(program, "lights[2].lightColor");
    specularLightUniform2 = glGetUniformLocation(program, "lights[2].specularLightColor");

    // Monk 1
    loadObjFile("Monk_Giveaway_Fixed.obj", monk1.meshVertices, monk1.meshIndices);
    monk1.diffuseTexture = loadGLTexture("Monk_D.tga");
    monk1.specularTexture = loadGLTexture("Monk_S.tga");
    monk1.normalTexture = loadGLTexture("Monk_N.tga");
    
    glUniform1i(normalTextureLoc, 0);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, monk1.normalTexture);
    
    glUniform1i(diffuseTexUniformLoc, 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, monk1.diffuseTexture);
    
    glUniform1i(specularTexUniformLoc, 2);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, monk1.specularTexture);
    
    glGenBuffers(1, &monk1.geometry.vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, monk1.geometry.vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPNTBTG) * monk1.meshVertices.size(), monk1.meshVertices.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &monk1.geometry.indexBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, monk1.geometry.indexBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * monk1.meshIndices.size(), monk1.meshIndices.data(), GL_STATIC_DRAW);
    
    monk1.geometry.numIndices = monk1.meshIndices.size();
    monk1.parent = nullptr;
    
    // Monk 2
    loadObjFile("Monk_Giveaway_Fixed.obj", monk2.meshVertices, monk2.meshIndices);
    monk2.diffuseTexture = loadGLTexture("Monk_D.tga");
    monk2.specularTexture = loadGLTexture("Monk_S.tga");
    monk2.normalTexture = loadGLTexture("Monk_N.tga");
    
    glUniform1i(normalTextureLoc, 0);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, monk2.normalTexture);
    
    glUniform1i(diffuseTexUniformLoc, 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, monk2.diffuseTexture);
    
    glUniform1i(specularTexUniformLoc, 2);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, monk2.specularTexture);
    
    glGenBuffers(1, &monk2.geometry.vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, monk2.geometry.vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPNTBTG) * monk2.meshVertices.size(), monk2.meshVertices.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &monk2.geometry.indexBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, monk2.geometry.indexBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * monk2.meshIndices.size(), monk2.meshIndices.data(), GL_STATIC_DRAW);
    
    monk2.geometry.numIndices = monk2.meshIndices.size();
    monk2.parent = &monk1;
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(750, 750);
    glutCreateWindow("CS-6533");
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    
    init();
    glutMainLoop();
    return 0;
}
