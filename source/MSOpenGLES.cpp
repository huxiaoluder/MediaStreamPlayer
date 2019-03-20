//
//  MSOpenGLES.cpp
//  MediaStreamPlayer
//
//  Created by xiaoming on 2019/3/12.
//  Copyright © 2019 freecoder. All rights reserved.
//

#include "MSOpenGLES.hpp"

using namespace MS;

GLuint
MSOpenGLES::loadShader(GLenum shaderType, const GLchar * const shaderStr) {
   
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
MSOpenGLES::linkProgram(const GLuint vertexShader, const GLuint fragmentShader) {
    
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
MSOpenGLES::bindFullViewportAttrBuffer() {
    // 纹理的坐标原点是在左下角的, 但纹理数据流是从左上角开始的, 为了不使用 CPU 调整数据, 直接修改顶点属性纹理坐标位置
    // 采用结构数组的方式组织数据, 前三列顶点坐标属性, 后两列 2D纹理坐标属性
    GLfloat coordinates[] = {
         1.0f,   1.0f,  0.0f,   1.0f,   0.0f,
        -1.0f,   1.0f,  0.0f,   0.0f,   0.0f,
        -1.0f,  -1.0f,  0.0f,   0.0f,   1.0f,
         1.0f,  -1.0f,  0.0f,   1.0f,   1.0f,
    };
    
    GLuint attrBuffer;
    glGenBuffers(1, &attrBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, attrBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coordinates), coordinates, GL_STATIC_DRAW);
    
    /*
     @Note: 使用顶点缓冲区, 可以减少 CPU 和 GPU 之间的内存交换, 效率高
     @Note: glGetAttribLocation(program, "着色器变量名") 可以获取到对应顶点属性的 index(location)
     @Note: 这里因为定制化 shader, 是直接固定写死的 index(location), 注: index 和 location 一定要映射准确
     */
    const GLuint vertexesAttrIdx  = 0; // 顶点坐标属性   index, 对应顶点着色器中 vtxCoord location
    const GLuint texturesAttrIdx  = 1; // 2D纹理坐标属性 index, 对应顶点着色其中 texCoord location
    const GLint vertexesAttrSize  = 3; // 顶点坐标属性基本元素数量
    const GLint texturesAttrSize  = 2; // 2D纹理坐标属性基本元素数量
    
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
    
    // 禁用顶点属性(这里顶点一直不变, 不用禁用)
//    glDisableVertexAttribArray(VERTEXES_ATTR_IDX);
//    glDisableVertexAttribArray(TEXTURES_ATTR_IDX);
    
    return attrBuffer;
}

GLuint
MSOpenGLES::generateEmptyTexture2D() {
    GLuint texture;
    glGenTextures(1, &texture);// 分配纹理对象
    glBindTexture(GL_TEXTURE_2D, texture);// 绑定纹理对象到 GL_TEXTURE_2D 目标
    /* @Note: 环绕方式 GL_CLAMP_TO_BORDER 超出的坐标为用户指定的边缘颜色, OpenGLES 不支持 */
    // GL_TEXTURE_WRAP_S 纹理水平环绕方式, GL_CLAMP_TO_EDGE 边缘拉伸
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // GL_TEXTURE_WRAP_T 纹理垂直环绕方式, GL_CLAMP_TO_EDGE 边缘拉伸
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // GL_TEXTURE_MAG_FILTER 纹理放大显示过滤器, GL_LINEAR 线性采样(提取对应顶点附近的纹素取平均值), GL_NEAREST 邻近采样(提取对应顶点最近的纹素值)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // GL_TEXTURE_MIN_FILTER 纹理缩小显示过滤器, GL_LINEAR_MIPMAP_LINEAR 在相邻两个多级渐远纹理之间线性插值, 再对插值纹素进行线性采样
    // @Note 使用 Mipmap 纹理链过滤器插值采样, 必须要 glGenerateMipmap(GL_TEXTURE_XD), 否则无法出图, 找不到对应级别的 Mipmap 纹理;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // 将纹理对象从 GL_TEXTURE_2D 目标解绑(0 为默认的,用户不可用的对象), 避免纹理对象混乱
    glBindTexture(GL_TEXTURE_2D, 0);
    return texture;
}

GLvoid
MSOpenGLES::commitTexture2DPixels(const GLuint  texture,
                                  const GLint   innerformat,
                                  const GLenum  pixelformat,
                                  const GLsizei width,
                                  const GLsizei height,
                                  const GLenum  type,
                                  const GLvoid * MSNonnull const pixels) {
    // 绑定纹理对象到 GL_TEXTURE_2D 目标
    glBindTexture(GL_TEXTURE_2D, texture);
    // CPU -交换数据-> GPU
    glTexImage2D(GL_TEXTURE_2D, // 纹理目标
                 0,             // Mipmap 级别
                 innerformat,   // 着色器中纹理数据格式
                 width,         // 纹理宽度
                 height,        // 纹理高度
                 0,             // 无用(历史遗留问题)
                 pixelformat,   // 像素数据格式
                 type,          // 像素数据类型
                 pixels);       // 像素数据
    // 手动让 GPU 为纹理生成多级渐远纹理链(按照 2 ^ n 倍缩小纹理, 只用于缩小, 放大基本无效果), GPU 根据视口大小选择合适的 Mip 级纹理进行采样渲染
    glGenerateMipmap(GL_TEXTURE_2D);
    // 将纹理对象从目标解绑(0 为默认的,用户不可用的对象), 避免纹理对象混乱
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLvoid
MSOpenGLES::activeTexture2DToProgram(const GLuint texture,
                                     const GLuint program,
                                     const GLenum textureUnit,
                                     const GLchar *const MSNonnull samplerName) {
    
    /*
     激活纹理单元, 纹理单元数量有上限(每个平台可能不同, 但是协议规定最少提供16个 GL_TEXTURE0 ~> GL_TEXTURE15), 它们是连续分配的
     一个纹理的默认纹理单元是 0，GL_TEXTURE0 它是默认的激活纹理单元, 如果只有一个纹理, 默认值被启用, 可以直接渲染
     */
    // 要使用其他的纹理单元, 需要在绑定纹理目标之前激活纹理单元
    glActiveTexture(textureUnit);
    
    // 绑定纹理对象到 GL_TEXTURE_2D 目标, 启用纹理单元
    glBindTexture(GL_TEXTURE_2D, texture);
    
    /*
     一个纹理的位置值对应一个纹理单元 GL_TEXTUREX(Texture Unit)
     使用glUniform1i，给纹理采样器分配一个位置值，为了在一个片段着色器中设置多个纹理
     */
    // 为目标纹理, 分配采样器位置, glGetUniformLocation(获取采样器在片段着色器中的位置)
    glUniform1i(glGetUniformLocation(program, samplerName), // 采样器在着色器中的位置
                textureUnit - GL_TEXTURE0); // 纹理单元的位置(纹理是单元连续分配的, 位置从0开始叠加)
}
