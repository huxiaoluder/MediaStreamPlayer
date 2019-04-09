//
//  MSGLESWrapper.cpp
//  MediaStreamPlayer
//
//  Created by xiaoming on 2019/3/1.
//  Copyright © 2019 freecoder. All rights reserved.
//

#include <string>
#include <sstream>
#include <fstream>
#include "MSGLHanlder.hpp"

using namespace std;
using namespace MS;

MSGLHanlder::MSGLHanlder(const char * const vshFilePath,
                         const char * const fshFilePath) {
    stringstream vshStream;
    stringstream fshStream;
    
    ifstream fileHandle;
    
    // 读取顶点着色器代码
    fileHandle.open(vshFilePath);
    vshStream << fileHandle.rdbuf();
    fileHandle.close();
    
    // 读取片段着色器代码
    fileHandle.open(fshFilePath);
    fshStream << fileHandle.rdbuf();
    fileHandle.close();
    
    string vshCode = vshStream.str();
    string fshCode = fshStream.str();
    
    GLuint vertexShader     = OpenGLES::loadShader(GL_VERTEX_SHADER,    vshCode.c_str());
    GLuint fragmentShader   = OpenGLES::loadShader(GL_FRAGMENT_SHADER,  fshCode.c_str());
    
    program = OpenGLES::linkProgram(vertexShader, fragmentShader);
    
    glUseProgram(program);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    attrBuffer = OpenGLES::bindFullViewportAttrBuffer();
    
    // 指定纹理在 OpenGL 中的内存对齐方式
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    yTexture = OpenGLES::generateEmptyTexture2D();
    uTexture = OpenGLES::generateEmptyTexture2D();
    vTexture = OpenGLES::generateEmptyTexture2D();
}

MSGLHanlder::~MSGLHanlder() {
    glDeleteProgram(program);
    glDeleteBuffers(1,  &attrBuffer);
    glDeleteTextures(1, &yTexture);
    glDeleteTextures(1, &uTexture);
    glDeleteTextures(1, &vTexture);
}

GLuint
MSGLHanlder::getProgram() const {
    return program;
}

GLuint
MSGLHanlder::getYtexture() const {
    return yTexture;
}

GLuint
MSGLHanlder::getUtexture() const {
    return uTexture;
}

GLuint
MSGLHanlder::getVtexture() const {
    return vTexture;
}
