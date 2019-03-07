//
//  MSGLWrapper.hpp
//  MediaStreamPlayer
//
//  Created by 胡校明 on 2019/3/1.
//  Copyright © 2019 freecoder. All rights reserved.
//

#ifndef MSGLWrapper_hpp
#define MSGLWrapper_hpp

#include <OpenGLES/ES3/glext.h>
#include <string>
#include <fstream>
#include "MSMacros.h"

#define VERTEXES_ATTR_IDX   0   // 顶点坐标属性   index, 对应顶点着色器中 vertexesCoord location
#define TEXTURES_ATTR_IDX   1   // 2D纹理坐标属性 index, 对应顶点找色其中 texturesCoord location

#define VERTEXES_ATTR_SIZE  3   // 顶点坐标属性基本元素数量
#define TEXTURES_ATTR_SIZE  2   // 2D纹理坐标属性基本元素数量

namespace MS {
    
    class MSGLWrapper {
        
        GLuint program;     // 二进制着色器程序对象
        GLuint attrBuffer;  // 顶点属性的缓冲区对象
        
    public:
        MSGLWrapper(const std::string &vshPath, const std::string &fshPath);
        ~MSGLWrapper();
        
        /**
         加载编译着色器
         
         @param shaderType 着色器类型 GL_..._SHADER
         @param shaderStr 着色器内容
         @return 着色器对象(free by caller)
         */
        static GLuint loadShader(const GLenum shaderType, const GLchar * const shaderStr);
        
        /**
         链接着色器程序
         
         @param vertexShader 顶点着色器
         @param fragmentShader 片段着色器
         @return 链接程序对象(free by caller)
         */
        static GLuint linkProgram(const GLuint vertexShader, const GLuint fragmentShader);
        
        /**
         绑定顶点属性缓冲区, 该函数只绑定 全窗口顶点坐标 和 全窗口纹理坐标, 并启用对应的属性

         @return 缓冲区对象(目标类型: GL_ARRAY_BUFFER)
         */
        static GLuint bindFullViewportAttrBuffer();
        
        static GLvoid renderBuffer();
    };

}

#endif /* MSGLWrapper_hpp */
