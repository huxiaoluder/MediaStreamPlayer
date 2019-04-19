/* 
  yuv420p.vsh
  MediaStreamPlayer

  Created by xiaoming on 2019/3/5.
  Copyright © 2019 freecoder. All rights reserved.
*/

#version 300 es

layout(location = 0) in vec3 vtxCoord;

layout(location = 1) in vec2 texCoord;

out vec2 textureCoord;

void main() {
    
    gl_Position  = vec4(vtxCoord, 1.0f);
    
    textureCoord = texCoord;
}
