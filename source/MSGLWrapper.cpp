//
//  MSGLESWrapper.cpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2019/3/1.
//  Copyright © 2019 freecoder. All rights reserved.
//

#include "MSGLWrapper.hpp"

using namespace MS;

GLuint
OpenGL::loadShader(GLenum shaderType, const GLchar * const shaderStr) {
    
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
OpenGL::linkProgram(const GLuint vertexShader, const GLuint fragmentShader) {
    
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
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}
