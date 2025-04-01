#version 460 core

in vec3 color;
in vec2 TexCoords;

uniform vec4 uniform_Color;
uniform sampler2D tex0;

out vec4 FragColor;

void main()
{
    vec4 texColor = texture(tex0, TexCoords);
    FragColor = texColor * vec4(color, 1.0);
}
