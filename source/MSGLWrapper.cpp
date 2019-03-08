//
//  MSGLESWrapper.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2019/3/1.
//  Copyright © 2019 freecoder. All rights reserved.
//

#include "MSGLWrapper.hpp"

using namespace std;
using namespace MS;

MSGLWrapper::MSGLWrapper(const string &vshPath, const string &fshPath) {
    string vsh;
    string fsh;
    
    char *readBuf = new char[1024];
    
    ifstream fileHandle;
    
    // 读取顶点着色器代码
    fileHandle.open(vshPath);
    fileHandle.read(readBuf, INT32_MAX);
    vsh.append(readBuf, fileHandle.gcount());
    fileHandle.close();
    
    // 读取片段着色器代码
    fileHandle.open(fshPath);
    fileHandle.read(readBuf, INT32_MAX);
    fsh.append(readBuf, fileHandle.gcount());
    fileHandle.close();
    
    delete [] readBuf;

    GLuint vertexShader     = loadShader(GL_VERTEX_SHADER,    vsh.c_str());
    GLuint fragmentShader   = loadShader(GL_FRAGMENT_SHADER,  fsh.c_str());
    
    program = linkProgram(vertexShader, fragmentShader);
    
    glUseProgram(program);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    attrBuffer = bindFullViewportAttrBuffer();
}

MSGLWrapper::~MSGLWrapper() {
    glDeleteProgram(program);
    glDeleteBuffers(1, &attrBuffer);
}

GLuint
MSGLWrapper::loadShader(GLenum shaderType, const GLchar * const shaderStr) {
    
    GLuint shader = 0;
    shader = glCreateShader(shaderType);
    
    glShaderSource(shader, 1, &shaderStr, nullptr);
    
    glCompileShader(shader);
    
    GLint compileState;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileState);
    
    if (compileState == GL_FALSE) {
        GLint infoLen;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        
        GLchar *infoLog = new GLchar[infoLen];
        glGetShaderInfoLog(shader, infoLen, nullptr, infoLog);
        
        ErrorLocationLog(infoLog);
        delete [] infoLog;
        glDeleteShader(shader);
        
        assert(compileState == 0);
    }
    return shader;
}

GLuint
MSGLWrapper::linkProgram(const GLuint vertexShader, const GLuint fragmentShader) {
    
    GLuint program = 0;
    program = glCreateProgram();
    
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    GLint linkState;
    glGetProgramiv(program, GL_LINK_STATUS, &linkState);
    
    if (linkState == GL_FALSE) {
        GLint infoLen;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
        
        GLchar *infoLog = new GLchar[infoLen];
        glGetProgramInfoLog(program, infoLen, nullptr, infoLog);
        
        ErrorLocationLog(infoLog);
        delete [] infoLog;
        glDeleteProgram(program);
        
        assert(linkState == 0);
    }
    
    return program;
}

GLuint
MSGLWrapper::bindFullViewportAttrBuffer() {
    GLfloat coordinates[] = { // 采用结构数组的方式组织数据, 前三列顶点坐标属性, 后两列 2D纹理坐标属性
        +1.0f,  +1.0f,  .0f,    .1f,    .0f,
        -1.0f,  +1.0f,  .0f,    .0f,    .0f,
        -1.0f,  -1.0f,  .0f,    .0f,    1.f,
        +1.0f,  -1.0f,  .0f,    1.f,    1.f,
    };
    
    GLuint attrBuffers[1] = {0};
    glGenBuffers(1, attrBuffers);
    glBindBuffer(GL_ARRAY_BUFFER, attrBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coordinates), coordinates, GL_STATIC_DRAW);
    
    /*
     @Note: 使用顶点缓冲区, 可以减少 CPU 和 GPU 之间的内存交换, 效率高
     @Note: glGetAttribLocation(program, "着色器变量名") 可以获取到对应顶点属性的 index(location)
     @Note: 这里因为定制化 shader, 是直接固定写死的 index(location), 注: index 和 location 一定要映射准确
     */
    const GLuint vertexesAttrIdx  = 0;    // 顶点坐标属性   index, 对应顶点着色器中 vertexesCoord location
    const GLuint texturesAttrIdx  = 1;    // 2D纹理坐标属性 index, 对应顶点着色其中 texturesCoord location
    const GLint vertexesAttrSize  = 3;    // 顶点坐标属性基本元素数量
    const GLint texturesAttrSize  = 2;    // 2D纹理坐标属性基本元素数量

    GLsizei vertexesLen = vertexesAttrSize * sizeof(GLfloat);
    GLsizei texturesLen = texturesAttrSize * sizeof(GLfloat);
    GLsizei attrStride  = vertexesLen + texturesLen;
    GLint64 offset = 0;
    
    // 启用顶点属性
    glEnableVertexAttribArray(vertexesAttrIdx);
    glEnableVertexAttribArray(texturesAttrIdx);
    
    // 顶点坐标属性 -映射-> 着色器顶点坐标变量
    glVertexAttribPointer(vertexesAttrIdx,  // 顶点属性 index, 对应着色器 layout(location = index) var
                          vertexesAttrSize, // 每个顶点属性含有的基本元素数量
                          GL_FLOAT,         // 顶点属性元素的类型
                          GL_FALSE,         // 是否规范化(非浮点数->浮点数的转化), 函数 glVertexAttribIPointer, 将所有数据当做整数对待
                          attrStride,       // 顶点属性的内存大小(Byte), 即获取下一个顶点属性的跨距(stride)
                          (const GLvoid *)offset);  // 如果使用((GPU运行内存)顶点缓冲区), 则为数据在顶点缓冲区的偏移量(偏移量用传值方式), 如果使用((CPU运行内存)内存缓冲区), 则为数据内存指针
    
    offset += vertexesLen;
    // 2D纹理坐标属性 -映射-> 着色器纹理坐标变量
    glVertexAttribPointer(texturesAttrIdx,
                          texturesAttrSize,
                          GL_FLOAT,
                          GL_FALSE,
                          attrStride,
                          (const GLvoid *)offset);
    
    // 禁用顶点属性
//    glDisableVertexAttribArray(VERTEXES_ATTR_IDX);
//    glDisableVertexAttribArray(TEXTURES_ATTR_IDX);
    
    return attrBuffers[0];
}
