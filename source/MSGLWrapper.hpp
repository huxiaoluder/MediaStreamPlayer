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
#include "MSMacros.h"

namespace MS {
    namespace OpenGL {
     
        /**
         加载并编译着色器

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
    }
}

#endif /* MSGLWrapper_hpp */
