#include <iostream>
#include <cstdlib>
#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_utils.h"

GLuint vbo_surface;
GLuint vbo_color;
GLint uniform_mvp;

GLuint program;

GLint attribute_coord3d;
GLint attribute_color;

GLfloat* points;
GLfloat* colors;

int screen_width = 800, screen_height = 800;

GLfloat cp[4][4][3] = {
    {{0.0, 0.0, 1.0},
     {0.0, 0.0, 0.66},
     {0.0, 0.0, 0.33},
     {0.0, 0.0, 0.0}},

    {{0.33, 0.0, 1.0},
     {0.33, 1.5, 0.66},
     {0.33, 1.5, 0.33},
     {0.33, 0.0, 0.0}},

    {{0.66, 0.0, 1.0},
     {0.66, 1.5, 0.66},
     {0.66, 1.5, 0.33},
     {0.66, 0.0, 0.0}},

    {{1.0, 0.0, 1.0},
     {1.0, 0.0, 0.66},
     {1.0, 0.0, 0.33},
     {1.0, 0.0, 0.0}}
};
int number_of_steps = 100; //numero de pasos

int factorial[4] = {1,1,2,6};

float binomial(int n, int i) {
    return 1.0*(factorial[n]/(factorial[i]* factorial[n-i]));
}
float bernstein(int n, int i, float t){
    return binomial(n,i) + powf(t,i) *powf(1-t,n-i);
}

bool init_resources(){

    float u = 0.0;
    float v;
    float bi, bj;
    float x,y,z;

    std::vector<float> p;

    for (int i = 0; i <= number_of_steps; i++) //recorrer u
    {
        for (int j = 0; j<= number_of_steps; j++) //recirrer v
        {
            for (int k = 0; k <=4; k++) //sumatorias de bezier
            {
                float bi = bernstein(3,k,u) ;//polinomio de bernstein para grado 3 k = i  con parametro u
                        
                for (int l = 0; l <=4; l++)
                    {
                        /* code */

                        float bj = bernstein(3,l,v);
                        x += bi * bj * cp[k][l][0];
                        y += bi * bj * cp[k][l][1];
                        z += bi * bj * cp[k][l][2];
                    }
            }
        }
        p.push_back(x);
        p.push_back(y);
        p.push_back(z);

        v+= 1/number_of_steps;
        u+= 1/number_of_steps;
    }

    points = new GLfloat[p.size()];

    for (int i = 0; i < p.size(); ++i)
    {
        points[i] = p[i];
    }

	GLint link_ok = GL_FALSE;
    GLuint vs, fs;
    if((vs = create_shader("basic3.v.glsl", GL_VERTEX_SHADER))==0) return false;
    if((fs = create_shader("basic3.f.glsl", GL_FRAGMENT_SHADER))==0) return false;

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if(!link_ok){
        std::cout << "Problemas con el Shader" << std::endl;
        return false;
    }

    attribute_coord3d = glGetAttribLocation(program, "coord3d");
    if(attribute_coord3d == -1){
        std::cout << "No se puede asociar el atributo coord3d" << std::endl;
        return false;
    }

    attribute_color = glGetAttribLocation(program, "color");
    if(attribute_color == -1){
        std::cout << "No se puede asociar el atributo color" << std::endl;
        return false;
    }

    uniform_mvp = glGetUniformLocation(program, "mvp");
    if(uniform_mvp == -1){
        std::cout << "No se puede asociar el uniform mvp" << std::endl;
        return false;
    }



    return true;
}

void onDisplay(){
	//Creamos matrices de modelo, vista y proyeccion
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    glm::mat4 view  = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(45.0f, 1.0f*screen_width/screen_height, 0.1f, 10.0f);
    glm::mat4 mvp = projection * view * model;

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glUseProgram(program);
    //Enviamos la matriz que debe ser usada para cada vertice
    glUniformMatrix4fv(uniform_mvp, 1, GL_FALSE, glm::value_ptr(mvp));

    

    glutSwapBuffers();
    
}

void onReshape(int w, int h){
    screen_width = w;
    screen_height = h;

    glViewport(0,0,screen_width, screen_height);
}

void free_resources(){
	glDeleteProgram(program);
    glDeleteBuffers(1, &vbo_surface);
    glDeleteBuffers(1, &vbo_color);

    delete[] points;
    delete[] colors;
}

int main(int argc, char* argv[]){
    glutInit(&argc, argv);
    glutInitContextVersion(2,0);
    glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(screen_width, screen_height);
    glutCreateWindow("OpenGL");

    GLenum glew_status = glewInit();
    if(glew_status != GLEW_OK){
        std::cout << "Error inicializando GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }

    if(!GLEW_VERSION_2_0){
        std::cout << "Tu tarjeta grafica no soporta OpenGL 2.0" << std::endl;
        exit(EXIT_FAILURE);
    }

    if(init_resources()){
        glutDisplayFunc(onDisplay);
        glutReshapeFunc(onReshape);
        glEnable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glutMainLoop();
    }

    free_resources();
    exit(EXIT_SUCCESS);
}
