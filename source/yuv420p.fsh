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

out vec4 fragColor;

void main() {
    fragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );
}
