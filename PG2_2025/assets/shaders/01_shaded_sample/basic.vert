#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal; // Changed from aColor
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aColor;


uniform mat4 uMVP;
uniform mat4 uModel;

out vec3 fragPos;
out vec3 normal;
out vec2 TexCoords;

void main()
{
    fragPos = vec3(uModel * vec4(aPos, 1.0));
    normal = mat3(transpose(inverse(uModel))) * aNormal;
    TexCoords = aTexCoords;

    gl_Position = uMVP * vec4(aPos, 1.0);
}
