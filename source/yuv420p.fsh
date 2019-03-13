/* 
  yuv420p.fsh
  ios_example

  Created by 胡校明 on 2019/3/5.
  Copyright © 2019 freecoder. All rights reserved.
*/

#version 300 es

// low 10位, mediump 16位, highp 32位
// float 只有在片段着色器中没有默认精度, 所以需要指定
// 当前为默认指定(全局有效), 单独指定将覆盖默认指定
precision highp float;

const mat3 yuv2rgbMat3 = mat3(1.164f,    1.164f,    1.164f,
                              0.0f,     -0.213f,    2.112f,
                              1.793f,   -0.533f,    0.0f);

uniform sampler2D ySampler2D;
uniform sampler2D uSampler2D;
uniform sampler2D vSampler2D;

in  vec2 textureCoord;

out vec4 fragColor;

void main() {
    // 内置函数 texture(sampler2D, textureCoord), 会自动将数据映射到(0 ~> 1.0)
    vec3 yuv = vec3(texture(ySampler2D, textureCoord).x,
                    texture(uSampler2D, textureCoord).x - 0.5,
                    texture(vSampler2D, textureCoord).x - 0.5);

    vec3 rgb = yuv2rgbMat3 * yuv;
    
    fragColor = vec4(rgb, 1.0f);
}
