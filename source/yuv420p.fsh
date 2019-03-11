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

in vec3 color;

out vec4 fragColor;

//uniform Sampler2D ytexture;
//uniform Sampler2D utexture;
//uniform Sampler2D vtexture;

mat4 convertMat4;

void main() {
    fragColor = vec4(color, 1.0f);
//    vec4(y,u,v,1) * convertMat4;
    
}
