#version 460 core

in vec3 fragPos;
in vec3 normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D tex0;
uniform float alpha;
uniform vec3 viewPos;

// === Ambient Light ===
uniform vec3 ambientLightColor;

// === Directional Light ===
struct DirectionalLight {
    vec3 direction;
    vec3 color;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform int numDirLights;
uniform DirectionalLight dirLights[10];

// === Point Light ===
struct PointLight {
    vec3 position;
    float constant;
    float linear;
    float quadratic;
    vec3 color;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform int numPointLights;
uniform PointLight pointLights[10];

// === Spot Light ===
struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    vec3 color;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform int numSpotLights;
uniform SpotLight spotLights[10];

// === Material ===
uniform vec3 ambientColor;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shininess;

void main()
{
    vec3 norm = normalize(normal);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec4 texColor = texture(tex0, TexCoords);
    vec3 result = vec3(0.0);

    // Global ambient light (optional override of ambientColor)
    result += ambientLightColor * ambientColor;

    // Directional lights
    for (int i = 0; i < numDirLights; ++i) {
        vec3 lightDir = normalize(-dirLights[i].direction);
        vec3 ambient = dirLights[i].ambient * ambientColor * dirLights[i].color;
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = dirLights[i].diffuse * diff * diffuseColor * dirLights[i].color;
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = dirLights[i].specular * spec * specularColor * dirLights[i].color;
        result += ambient + diffuse + specular;
    }

    // Point lights
    for (int i = 0; i < numPointLights; ++i) {
        vec3 lightDir = normalize(pointLights[i].position - fragPos);
        float distance = length(pointLights[i].position - fragPos);
        float attenuation = 1.0 / (pointLights[i].constant + pointLights[i].linear * distance +
                                   pointLights[i].quadratic * (distance * distance));

        vec3 ambient = pointLights[i].ambient * ambientColor * pointLights[i].color;
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = pointLights[i].diffuse * diff * diffuseColor * pointLights[i].color;
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = pointLights[i].specular * spec * specularColor * pointLights[i].color;

        result += attenuation * (ambient + diffuse + specular);
    }

    // Spotlights
    for (int i = 0; i < numSpotLights; ++i) {
        vec3 lightDir = normalize(spotLights[i].position - fragPos);
        float theta = dot(lightDir, normalize(-spotLights[i].direction));
        float epsilon = spotLights[i].cutOff - spotLights[i].outerCutOff;
        float intensity = clamp((theta - spotLights[i].outerCutOff) / epsilon, 0.0, 1.0);

        vec3 ambient = spotLights[i].ambient * ambientColor * spotLights[i].color;
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = spotLights[i].diffuse * diff * diffuseColor * spotLights[i].color;
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
        vec3 specular = spotLights[i].specular * spec * specularColor * spotLights[i].color;

        result += intensity * (ambient + diffuse + specular);
    }

    vec3 litColor = result * texColor.rgb;
    FragColor = vec4(litColor, texColor.a * alpha);
}
