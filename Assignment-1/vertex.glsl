attribute vec4 position;
attribute vec4 color;
attribute vec2 texCoord;

varying vec4 varyingColor;
varying vec2 varyingTexCoord;

uniform vec2 modelPosition;

void main() {
    //varyingColor = color;
    varyingTexCoord = texCoord;
    gl_Position = vec4(modelPosition.x, modelPosition.y, 0.0, 0.0) + position;
}