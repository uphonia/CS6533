/*
attribute vec4 position;
attribute vec4 color;
attribute vec2 texCoord;

varying vec4 varyingColor;
varying vec2 varyingTexCoord;

uniform vec2 modelPosition;

uniform mat4 modelViewMatrix;

uniform mat4 projectionMatrix;

void main() {
    //varyingColor = color;
    //varyingTexCoord = texCoord;
    //gl_Position = vec4(modelPosition.x, modelPosition.y, 0.0, 0.0) + position;
    
    //gl_Position = modelViewMatrix * position;
    
    varyingColor = color;
    gl_Position = projectionMatrix * modelViewMatrix * position;
}
*/

attribute vec4 position;
attribute vec4 normal;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 normalMatrix;

varying vec4 varyingNormal;

void main() {
    varyingNormal = normalize(normalMatrix * normal);
    gl_Position = projectionMatrix * modelViewMatrix * position;
}
