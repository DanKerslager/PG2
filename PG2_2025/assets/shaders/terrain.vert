#version 460 core

layout(location = 0) in vec3 aPos;       // Vertex Position
layout(location = 1) in vec3 aNormal;    // Vertex Normal
layout(location = 2) in vec2 aTexCoords; // Texture Coordinates
layout(location = 3) in vec3 aColor;


uniform mat4 uMVP;  // Model-View-Projection Matrix
uniform mat4 uModel; // Model Matrix

out vec3 FragPos;   // Pass position to fragment shader
out vec3 Normal;    // Pass normal
out vec2 TexCoords; // Pass texture coordinates

void main()
{
    FragPos = vec3(uModel * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(uModel))) * aNormal; // Correctly transform normal
    TexCoords = aTexCoords;
    
    gl_Position = uMVP * vec4(aPos, 1.0);
}
