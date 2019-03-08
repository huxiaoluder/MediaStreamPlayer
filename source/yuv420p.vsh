/* 
  yuv420p.vsh
  ios_example

  Created by 胡校明 on 2019/3/5.
  Copyright © 2019 freecoder. All rights reserved.
*/

#version 300 es

layout(location = 0) in vec4 vertexesCoord;

layout(location = 1) in vec2 texturesCoord;

void main() {
    gl_Position = vertexesCoord;
}
