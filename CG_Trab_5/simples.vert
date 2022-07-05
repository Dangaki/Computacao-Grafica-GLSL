#version 330 core
layout (location = 0) in vec3 entrada_pos;
layout (location = 1) in vec2 entrada_pos_text;
layout (location = 2) in vec3 entrada_normal;

uniform vec3 movimento;
uniform vec3 rotacao;
uniform float escala;

uniform mat4 projecao;

out vec2 pos_text;
out vec3 normal;
out vec3 posicao;

vec3 rotacionarX(vec3 p);
vec3 rotacionarY(vec3 p);
vec3 rotacionarZ(vec3 p);

void main()
{   
    //Operações dos Vertices
    vec3 new_posicao = entrada_pos;

    new_posicao = rotacionarX(new_posicao);
    new_posicao = rotacionarY(new_posicao);
    new_posicao = rotacionarZ(new_posicao);

    new_posicao = new_posicao + movimento;

    new_posicao = new_posicao * escala;

    //Saida
    pos_text = entrada_pos_text;
    normal = entrada_normal;
    posicao = new_posicao;

    //Escrita do Vertices
    gl_Position = projecao * vec4(new_posicao, 1.0);

} 

vec3 rotacionarX(vec3 p){
    
    vec3 retorno = p;

	retorno.y = cos(rotacao.x) * p.y - sin(rotacao.x) * p.z;
    retorno.z = sin(rotacao.x) * p.y + cos(rotacao.x) * p.z;

    return retorno;
}

vec3 rotacionarY(vec3 p){

    vec3 retorno = p;

	retorno.x =  cos(rotacao.y) * p.x + sin(rotacao.y) * p.z;
	retorno.z = -sin(rotacao.y) * p.x + cos(rotacao.y) * p.z;

    return retorno;
}

vec3 rotacionarZ(vec3 p){

    vec3 retorno = p;

	retorno.x = cos(rotacao.z) * p.x - sin(rotacao.z) * p.y;
	retorno.y = sin(rotacao.z) *p.x + cos(rotacao.z) *p.y;

    return retorno;
}