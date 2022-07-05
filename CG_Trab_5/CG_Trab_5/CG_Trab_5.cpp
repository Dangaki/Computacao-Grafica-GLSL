/*
    Alunos:
            Daniel Akira Nakamura Gullich
            Lucas Gabriel Grutka Telles
            Eduardo Oliveira

    Comandos:
            Esc - Sair
            WASD QE - Movimentar (Translação)
            IJKL UO - Rotacionar
            VB		- Escalar
*/

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <glew.h>
#include <GL/glut.h>
#include <chrono>
#include <fstream>
#include <vector>
#include <string>


std::string nome_obj = "data/george-washington.obj";

struct Vertice {
    double x, y, z;
};

struct Vertice_t {
    double x, y;
};

struct Face {
    std::vector<int> vertices_indice;
    std::vector<int> vertices_indice_t;
    std::vector<int> vertices_indice_n;
};

struct Objeto {
    Vertice posicao;
    Vertice escala;
    double rotacao[3];
    double matriz_rotacao[3][3];

    std::vector<Vertice> vertices;
    std::vector<Vertice> vertices_n;
    std::vector<Vertice_t> vertices_t;
    std::vector<Face> faces;
};

Objeto objeto;

GLuint v, f;
GLuint p;

GLuint VAO, VBO;

static unsigned int texturas[2];

float intensidadeLuzAmbiente = 0.1;
float intensidadeLuzDifusa = 0.9;
float corLuz[3] = { 1, 1, 1 };
float posLuz[3] = { 0, 1, 0.25 };

float posicao[3] = { 0, 0, 0 };
float rotacao[3] = { 0, 0, 0 };
float escala = 1;

struct BitMapFile
{
    int sizeX;
    int sizeY;
    unsigned char* data;
};

// Funciona somente com bmp de 24 bits
BitMapFile* getBMPData(std::string filename)
{
    BitMapFile* bmp = new BitMapFile;
    unsigned int size, offset, headerSize;

    // Ler o arquivo de entrada
    std::ifstream infile(filename.c_str(), std::ios::binary);

    // Pegar o ponto inicial de leitura
    infile.seekg(10);
    infile.read((char*)&offset, 4);

    // Pegar o tamanho do cabeçalho do bmp.
    infile.read((char*)&headerSize, 4);

    // Pegar a altura e largura da imagem no cabeçalho do bmp.
    infile.seekg(18);
    infile.read((char*)&bmp->sizeX, 4);
    infile.read((char*)&bmp->sizeY, 4);

    // Alocar o buffer para a imagem.
    size = bmp->sizeX * bmp->sizeY * 24;
    bmp->data = new unsigned char[size];

    // Ler a informação da imagem.
    infile.seekg(offset);
    infile.read((char*)bmp->data, size);

    // Reverte a cor de bgr para rgb
    int temp;
    for (int i = 0; i < size; i += 3)
    {
        temp = bmp->data[i];
        bmp->data[i] = bmp->data[i + 2];
        bmp->data[i + 2] = temp;
    }

    return bmp;
}

void loadExternalTextures(std::string nome_arquivo, int id_textura)
{
    BitMapFile* image[1];

    image[0] = getBMPData(nome_arquivo);

    glBindTexture(GL_TEXTURE_2D, texturas[id_textura]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image[0]->sizeX, image[0]->sizeY, 0,
        GL_RGB, GL_UNSIGNED_BYTE, image[0]->data);
}

char* readStringFromFile(char* fn) {

    FILE* fp;
    char* content = NULL;
    int count = 0;

    if (fn != NULL) {
        fopen_s(&fp, fn, "rt");

        if (fp != NULL) {

            fseek(fp, 0, SEEK_END);
            count = ftell(fp);
            rewind(fp);

            if (count > 0) {
                content = (char*)malloc(sizeof(char) * (count + 1));
                count = fread(content, sizeof(char), count, fp);
                content[count] = '\0';
            }
            fclose(fp);
        }
    }
    return content;
}

Vertice criar_vertice(double pos_x, double pos_y, double pos_z) {
    Vertice novo_v;
    novo_v.x = pos_x;
    novo_v.y = pos_y;
    novo_v.z = pos_z;

    return novo_v;
}

Vertice_t criar_vertice_t(double pos_x, double pos_y) {
    Vertice_t novo_v;
    novo_v.x = pos_x;
    novo_v.y = pos_y;

    return novo_v;
}

std::vector<std::string> tokenizar(std::string pivo) {

    std::vector<std::string> pivo_split;
    int inicio = 0;
    int final = pivo.find("/");

    while (final != -1) {
        pivo_split.push_back(pivo.substr(inicio, final - inicio));
        inicio = final + 1;
        final = pivo.find("/", inicio);
    }

    pivo_split.push_back(pivo.substr(inicio, final - inicio));

    return pivo_split;
}

Face criar_face(std::string x, std::string y, std::string z) {

    Face nova_f;
    std::vector<std::string> pivo_split;

    //Para os indices dos vertices 
    pivo_split = tokenizar(x);
    nova_f.vertices_indice.push_back((pivo_split[0] == "") ? 0 : std::stoi(pivo_split[0]) - 1);
    nova_f.vertices_indice_t.push_back((pivo_split[1] == "") ? 0 : std::stoi(pivo_split[1]) - 1);
    nova_f.vertices_indice_n.push_back((pivo_split[2] == "") ? 0 : std::stoi(pivo_split[2]) - 1);

    //Para os indices dos vertices da textura
    pivo_split = tokenizar(y);
    nova_f.vertices_indice.push_back((pivo_split[0] == "") ? 0 : std::stoi(pivo_split[0]) - 1);
    nova_f.vertices_indice_t.push_back((pivo_split[1] == "") ? 0 : std::stoi(pivo_split[1]) - 1);
    nova_f.vertices_indice_n.push_back((pivo_split[2] == "") ? 0 : std::stoi(pivo_split[2]) - 1);

    //Para os indices dos vertices da normal
    pivo_split = tokenizar(z);
    nova_f.vertices_indice.push_back((pivo_split[0] == "") ? 0 : std::stoi(pivo_split[0]) - 1);
    nova_f.vertices_indice_t.push_back((pivo_split[1] == "") ? 0 : std::stoi(pivo_split[1]) - 1);
    nova_f.vertices_indice_n.push_back((pivo_split[2] == "") ? 0 : std::stoi(pivo_split[2]) - 1);

    return nova_f;
}

Objeto loadObj(std::string fname) {

    Objeto novo_obj;

    double x, y, z;

    std::ifstream arquivo(fname);

    if (!arquivo.is_open()) {
        std::cout << "Arquivo nao encontrado\n";
        exit(1);
    }
    else {
        std::string tipo;
        while (arquivo >> tipo)
        {
            if (tipo == "v")
            {
                double x, y, z;
                arquivo >> x >> y >> z;
                novo_obj.vertices.push_back(criar_vertice(x, y, z));
            }

            if (tipo == "f")
            {
                std::string x, y, z;
                arquivo >> x >> y >> z;
                novo_obj.faces.push_back(criar_face(x, y, z));
            }

            if (tipo == "vn")
            {
                double x, y, z;
                arquivo >> x >> y >> z;
                novo_obj.vertices_n.push_back(criar_vertice(x, y, z));
            }

            if (tipo == "vt")
            {
                double x, y;
                arquivo >> x >> y;
                novo_obj.vertices_t.push_back(criar_vertice_t(x, y));
            }

        }
    }

    novo_obj.posicao.x = 0.0f;
    novo_obj.posicao.y = 0.0f;
    novo_obj.posicao.z = 0.0f;

    novo_obj.escala.x = 1;
    novo_obj.escala.y = 1;
    novo_obj.escala.z = 1;

    novo_obj.rotacao[0] = 0;
    novo_obj.rotacao[1] = 0;
    novo_obj.rotacao[2] = 0;

    return novo_obj;
}

int teste1 = 0;
//Tranforma OBJ em um Vector
std::vector<float> converterObjVector() {
 
    std::vector<float> objeto_v;

    for (int i = 0; i < objeto.faces.size(); i++) {
        //Utiliza o indice das Faces
        std::vector<int> face   = objeto.faces[i].vertices_indice;
        std::vector<int> face_t = objeto.faces[i].vertices_indice_t;
        std::vector<int> face_n = objeto.faces[i].vertices_indice_n;
        
        //Existem 3 vertices para cada face
        for (int vertice = 0; vertice < 3; vertice++){

            //Posição do vertice
            objeto_v.push_back(objeto.vertices[face[vertice]].x);
            objeto_v.push_back(objeto.vertices[face[vertice]].y);
            objeto_v.push_back(objeto.vertices[face[vertice]].z);

            //Posição da textura
            objeto_v.push_back(objeto.vertices_t[face_t[vertice]].x);
            objeto_v.push_back(objeto.vertices_t[face_t[vertice]].y);

            //Posição da normal
            objeto_v.push_back(objeto.vertices_n[face_n[vertice]].x);
            objeto_v.push_back(objeto.vertices_n[face_n[vertice]].y);
            objeto_v.push_back(objeto.vertices_n[face_n[vertice]].z);
        }

        if (teste1 != 3) {
            for (int vertice = 0; vertice < 3; vertice++) {
                std::cout << objeto.vertices[face[vertice]].x << "|" << objeto.vertices[face[vertice]].y << "|" << objeto.vertices[face[vertice]].z << " ||| "
                    << objeto.vertices_t[face_t[vertice]].x << "|" << objeto.vertices_t[face_t[vertice]].y << " ||| "
                    << objeto.vertices_n[face_n[vertice]].x << "|" << objeto.vertices_n[face_n[vertice]].y << "|" << objeto.vertices_n[face_n[vertice]].z << std::endl;
            }

            teste1++;
        }

    }

    return objeto_v;
}

void setShaders() {
    char* vs = NULL, * fs = NULL, * fs2 = NULL;

    glewInit();

    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);

    char vertex_shader[] = "simples.vert";
    char fragment_shader[] = "simples.frag";
    vs = readStringFromFile(vertex_shader);
    fs = readStringFromFile(fragment_shader);

    const char* vv = vs;
    const char* ff = fs;

    glShaderSource(v, 1, &vv, NULL);
    glShaderSource(f, 1, &ff, NULL);

    free(vs); free(fs);

    glCompileShader(v);
    glCompileShader(f);

    int result;
    glGetShaderiv(v, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int tam;
        glGetShaderiv(v, GL_INFO_LOG_LENGTH, &tam);
        char* mensagem = new char[tam];
        glGetShaderInfoLog(v, tam, &tam, mensagem);
        std::cout << mensagem << std::endl;
    }

    glGetShaderiv(f, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int tam;
        glGetShaderiv(f, GL_INFO_LOG_LENGTH, &tam);
        char* mensagem = new char[tam];
        glGetShaderInfoLog(f, tam, &tam, mensagem);
        std::cout << mensagem << std::endl;
    }

    p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);

    glLinkProgram(p);

    std::vector<float> objeto_v = converterObjVector();

    std::vector<float> vertices_v = {
        0.5f,  0.5f, 0.0f,   1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.0f,   1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
       -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
       -0.5f,  0.5f, 0.0f,   0.0f, 1.0f,   0.0f, 0.0f, 1.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices_v.size() * sizeof(vertices_v), &vertices_v[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glUseProgram(p);

}

void display(void)
{
    glClearColor(0.5, 0.5, 0.5, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    double w = glutGet(GLUT_WINDOW_WIDTH);
    double h = glutGet(GLUT_WINDOW_HEIGHT);

    glColor3f(1, 0, 0);

    //Iluminação
    int id_int_luz_amb = glGetUniformLocation(p, "luz_int_amb");
    glUniform1f(id_int_luz_amb, intensidadeLuzAmbiente);

    int id_int_luz_dif = glGetUniformLocation(p, "luz_int_dif");
    glUniform1f(id_int_luz_dif, intensidadeLuzDifusa);

    int id_cor_luz = glGetUniformLocation(p, "cor_luz");
    glUniform3f(id_cor_luz, corLuz[0], corLuz[1], corLuz[2]);

    int id_pos_luz = glGetUniformLocation(p, "pos_luz");
    glUniform3f(id_pos_luz, posLuz[0], posLuz[1], posLuz[2]);

    //Movimento
    int id_posicao_m = glGetUniformLocation(p, "movimento");
    glUniform3f(id_posicao_m, posicao[0], posicao[1], posicao[2]);

    int id_rotacao = glGetUniformLocation(p, "rotacao");
    glUniform3f(id_rotacao, rotacao[0], rotacao[1], rotacao[2]);

    int id_escala = glGetUniformLocation(p, "escala");
    glUniform1f(id_escala, escala);


    //Textura
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texturas[0]);
    int sp_texture = glGetUniformLocation(p, "textura");
    glUniform1i(sp_texture, 0);

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, texturas[1]);
    int sp_texture_normal = glGetUniformLocation(p, "textura_normal");
    glUniform1i(sp_texture_normal, 1);


    glBindTexture(GL_TEXTURE_2D, texturas[1]);
    glBindVertexArray(VAO);
    glDrawArrays(GL_QUADS, 0, 4);

    glFlush();
    glutSwapBuffers();
    glutPostRedisplay();
}


void key(unsigned char k, int x, int y)
{
    /*
    if (k == 'q') exit(0);
    if (k == 'z')
        intensidadeLuzAmbiente -= 0.05;
    else if(k == 'x')
        intensidadeLuzAmbiente += 0.05;

    if (k == 'a')
        posLuz[0] -= 0.05;
    else if (k == 'd')
        posLuz[0] += 0.05;

    if (k == 'w')
        posLuz[1] += 0.05;
    else if (k == 's')
        posLuz[1] -= 0.05;
     */
    switch (k) {
    case 27:
        exit(0);
        break;
        //Movimento
    case 'w':
        posicao[1] += 0.05;
        break;
    case 's':
        posicao[1] -= 0.05;
        break;
    case 'd':
        posicao[0] += 0.05;
        break;
    case 'a':
        posicao[0] -= 0.05;
        break;
    case 'q':
        posicao[2] += 0.05;
        break;
    case 'e':
        posicao[2] -= 0.05;
        break;

        // Rotação
    case 'i':
        rotacao[0] -= 0.1;
        break;
    case 'k':
        rotacao[0] += 0.1;
        break;
    case 'j':
        rotacao[1] -= 0.1;
        break;
    case 'l':
        rotacao[1] += 0.1;
        break;
    case 'u':
        rotacao[2] -= 0.1;
        break;
    case 'o':
        rotacao[2] += 0.1;
        break;

        // Escala
    case 'v':
        escala = escala * 2;
        break;
    case 'b':
        escala = escala * 0.5;
        break;
    }
}

void reshape(int width, int height)
{
    
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();


    //gluPerspective(60, (GLfloat)width / (GLfloat)height, 0.1, 5000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
}

void setupRC() {

    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLfloat diffuseLight[] = { 0.7, 0.7, 0.7, 1.0 };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glEnable(GL_LIGHT0);

    GLfloat ambientLight[] = { 0.05, 0.05, 0.05, 1.0 };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);

    GLfloat specularLight[] = { 0.7, 0.7, 0.7, 1.0 };
    GLfloat spectre[] = { 1.0, 1.0, 1.0, 1.0 };

    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spectre);
    glMateriali(GL_FRONT, GL_SHININESS, 128);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glEnable(GL_LIGHTING);
}

void init()
{
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    setupRC();

    glGenTextures(2, texturas);
    loadExternalTextures("Textures/brickwall.bmp", 0);
    loadExternalTextures("Textures/brickwall_normal.bmp", 1);
    glEnable(GL_TEXTURE_2D);


    //comente essa linha para desabilitar os shaders
    setShaders();
}

int main(int argc, char* argv[])
{
    objeto = loadObj(nome_obj);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(640, 480);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Shader");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(key);

    glutMainLoop();

    return 0;
}