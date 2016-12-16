varying vec3 varyingNormal;
varying vec3 varyingPosition;
varying vec2 varyingTexCoord;
varying vec2 texCoordVar;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D screenFramebuffer;

uniform sampler2D normalTexture;
uniform sampler2D lookupTable;

varying mat3 varyingTBNMatrix;

struct Light {
    vec3 lightPosition;
    vec3 lightColor;
    vec3 specularLightColor;
};

uniform Light lights[2];

float attenuate(float dist, float a, float b) {
    return 1.0 / (1.0 + a*dist + b*dist*dist);
}

vec4 lookup(vec4 textureColor, sampler2D lookupTable) {
    float blueColor = textureColor.b * 63.0;
    
    vec2 quad1;
    quad1.y = floor(floor(blueColor) / 8.0);
    quad1.x = floor(blueColor) - (quad1.y * 8.0);
    
    vec2 quad2;
    quad2.y = floor(ceil(blueColor) / 8.0);
    quad2.x = ceil(blueColor) - (quad2.y * 8.0);
    
    vec2 texPos1;
    texPos1.x = (quad1.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);
    texPos1.y = (quad1.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);
    
    vec2 texPos2;
    texPos2.x = (quad2.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);
    texPos2.y = (quad2.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);
    
    vec4 newColor1 = texture2D(lookupTable, texPos1);
    vec4 newColor2 = texture2D(lookupTable, texPos2);
    
    vec4 newColor = mix(newColor1, newColor2, fract(blueColor));
    
    return newColor;
}

void main() {
    vec3 diffuseColor = vec3(1.0, 0.0, 0.0);
    vec3 specularColor = vec3(1.0, 0.0, 0.0);
    
    vec3 textureNormal = normalize((texture2D(normalTexture, varyingTexCoord).xyz * 2.0)-1.0);
    textureNormal = normalize(varyingTBNMatrix * textureNormal);
    
    for(int i = 0; i < 2; i++) {
        vec3 lightDirection = -normalize(varyingPosition-lights[i].lightPosition);
        float diffuse = max(0.0, dot(varyingNormal, lightDirection));
        
        float attenuation = attenuate(distance(varyingPosition, lights[i].lightPosition) / 5.0, 2.7, 5.0);
        diffuseColor += (lights[i].lightColor * diffuse) * attenuation;
        
        vec3 v = normalize(-varyingPosition);
        vec3 h = normalize(v + lightDirection);
        
        float specular = pow(max(0.0, dot(h, varyingNormal)), 64.0);
        specularColor += lights[i].specularLightColor * specular * attenuation;
    }

    // HDR Tone Mapping
    const float gamma = 2.2;
    vec3 hdrColor = texture2D( screenFramebuffer, varyingTexCoord).rgb;
    vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
    mapped = pow(mapped, vec3(gamma));
    
    vec3 intensity = (texture2D(diffuseTexture, varyingTexCoord).xyz * diffuseColor) + (specularColor * texture2D(specularTexture, varyingTexCoord).x) +
    (texture2D(normalTexture, varyingTexCoord).x);
    vec4 newColor = lookup(vec4(specularColor.xyz, 1.0), lookupTable);
    
    gl_FragColor = (vec4(intensity.xyz, 1.0) * newColor + vec4(mapped.xyz, 1.0)) ;
     
}

