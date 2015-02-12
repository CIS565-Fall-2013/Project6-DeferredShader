#include "GLApp.h"
#include "Camera.h"
#include "Utility.h"
#include "EventHandlers.h"

#include <GL/glew.h>
#include "GLFW/glfw3.h"
#include "SOIL/SOIL.h"
#include "tiny_obj_loader.h"
#include <glm/gtc/matrix_projection.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

using glm::vec4;
using glm::vec3;
using glm::vec2;
using glm::mat4;

GLApp* GLApp::m_singleton = nullptr;

namespace
{
    void uploadMesh(const mesh_t & mesh, device_mesh_t& out)
    {
        //Allocate vertex array
        //Vertex arrays encapsulate a set of generic vertex 
        //attributes and the buffers they are bound to
        //Different vertex array per mesh.
        glGenVertexArrays(1, &(out.vertex_array));
        glBindVertexArray(out.vertex_array);

        //Allocate vbos for data
        glGenBuffers(1, &(out.vbo_vertices));
        glGenBuffers(1, &(out.vbo_normals));
        glGenBuffers(1, &(out.vbo_indices));
        glGenBuffers(1, &(out.vbo_texcoords));

        //Upload vertex data
        glBindBuffer(GL_ARRAY_BUFFER, out.vbo_vertices);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size()*sizeof(vec3),
            &mesh.vertices[0], GL_STATIC_DRAW);
        glVertexAttribPointer(mesh_attributes::POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(mesh_attributes::POSITION);

        //Upload normal data
        glBindBuffer(GL_ARRAY_BUFFER, out.vbo_normals);
        glBufferData(GL_ARRAY_BUFFER, mesh.normals.size()*sizeof(vec3),
            &mesh.normals[0], GL_STATIC_DRAW);
        glVertexAttribPointer(mesh_attributes::NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(mesh_attributes::NORMAL);

        //Upload texture coord data
        glBindBuffer(GL_ARRAY_BUFFER, out.vbo_texcoords);
        glBufferData(GL_ARRAY_BUFFER, mesh.texcoords.size()*sizeof(vec2),
            &mesh.texcoords[0], GL_STATIC_DRAW);
        glVertexAttribPointer(mesh_attributes::TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(mesh_attributes::TEXCOORD);

        //indices
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out.vbo_indices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size()*sizeof(GLushort),
            &mesh.indices[0], GL_STATIC_DRAW);
        out.num_indices = mesh.indices.size();
        //Unplug Vertex Array
        glBindVertexArray(0);

        out.texname = mesh.texname;
        out.color = mesh.color;
    }
}

GLApp::GLApp(uint32_t width, uint32_t height, std::string windowTitle)
    : m_startTime(0), 
    m_currentTime(0),
    m_currentFrame(0),
    m_displayType(DISPLAY_TOTAL),
    m_width(width),
    m_height(height),
    m_bloomEnabled(true),
    m_toonEnabled(false),
    m_DOFEnabled(false),
    m_DOFDebug(true),
    m_scissorEnabled(true),
    m_mouseCaptured(true),
    mouse_dof_x(0),
    mouse_dof_y(0),
    m_farPlane(5000.0f),
    m_nearPlane(0.1f),
    m_randomNormalTexture(0),
    m_randomScalarTexture(0),
    m_depthTexture(0),
    m_normalTexture(0),
    m_positionTexture(0),
    m_colorTexture(0),
    m_postTexture(0),
    m_glowmaskTexture(0),
    m_passProg(0),
    m_pointProg(0),
    m_ambientProg(0),
    m_diagnosticProg(0),
    m_postProg(0), 
    m_windowTitle(windowTitle)
{
    vec3 tilt(1.0f, 0.0f, 0.0f);
    mat4 tilt_mat = mat4();
    mat4 scale_mat = glm::scale(mat4(), vec3(0.01));
    m_world = tilt_mat * scale_mat;

    m_cam = new Camera(vec3(0, 2, 0), glm::normalize(vec3(0, 0, -1)), glm::normalize(vec3(0, 1, 0)));

    m_invHeight = 1.0f / (m_height - 1);
    m_invWidth = 1.0f / (m_width - 1);

    m_lastX = width / 2.0;
    m_lastY = height / 2.0;
}

GLApp::~GLApp()
{
    delete m_cam;
}

void GLApp::initMesh(std::vector<tinyobj::shape_t>& shapes)
{
    for (std::vector<tinyobj::shape_t>::iterator it = shapes.begin();
        it != shapes.end(); ++it)
    {
        tinyobj::shape_t shape = *it;
        int totalsize = shape.mesh.indices.size() / 3;
        int f = 0;
        while (f < totalsize)
        {
            mesh_t mesh;
            int process = std::min(10000, totalsize - f);
            int point = 0;
            for (int i = f; i<process + f; i++)
            {
                int idx0 = shape.mesh.indices[3 * i];
                int idx1 = shape.mesh.indices[3 * i + 1];
                int idx2 = shape.mesh.indices[3 * i + 2];
                vec3 p0 = vec3(shape.mesh.positions[3 * idx0],
                    shape.mesh.positions[3 * idx0 + 1],
                    shape.mesh.positions[3 * idx0 + 2]);
                vec3 p1 = vec3(shape.mesh.positions[3 * idx1],
                    shape.mesh.positions[3 * idx1 + 1],
                    shape.mesh.positions[3 * idx1 + 2]);
                vec3 p2 = vec3(shape.mesh.positions[3 * idx2],
                    shape.mesh.positions[3 * idx2 + 1],
                    shape.mesh.positions[3 * idx2 + 2]);

                mesh.vertices.push_back(p0);
                mesh.vertices.push_back(p1);
                mesh.vertices.push_back(p2);

                if (shape.mesh.normals.size() > 0)
                {
                    mesh.normals.push_back(vec3(shape.mesh.normals[3 * idx0],
                        shape.mesh.normals[3 * idx0 + 1],
                        shape.mesh.normals[3 * idx0 + 2]));
                    mesh.normals.push_back(vec3(shape.mesh.normals[3 * idx1],
                        shape.mesh.normals[3 * idx1 + 1],
                        shape.mesh.normals[3 * idx1 + 2]));
                    mesh.normals.push_back(vec3(shape.mesh.normals[3 * idx2],
                        shape.mesh.normals[3 * idx2 + 1],
                        shape.mesh.normals[3 * idx2 + 2]));
                }
                else
                {
                    vec3 norm = glm::normalize(glm::cross(glm::normalize(p1 - p0), glm::normalize(p2 - p0)));
                    mesh.normals.push_back(norm);
                    mesh.normals.push_back(norm);
                    mesh.normals.push_back(norm);
                }

                if (shape.mesh.texcoords.size() > 0)
                {
                    mesh.texcoords.push_back(vec2(shape.mesh.positions[2 * idx0],
                        shape.mesh.positions[2 * idx0 + 1]));
                    mesh.texcoords.push_back(vec2(shape.mesh.positions[2 * idx1],
                        shape.mesh.positions[2 * idx1 + 1]));
                    mesh.texcoords.push_back(vec2(shape.mesh.positions[2 * idx2],
                        shape.mesh.positions[2 * idx2 + 1]));
                }
                else
                {
                    vec2 tex(0.0);
                    mesh.texcoords.push_back(tex);
                    mesh.texcoords.push_back(tex);
                    mesh.texcoords.push_back(tex);
                }
                mesh.indices.push_back(point++);
                mesh.indices.push_back(point++);
                mesh.indices.push_back(point++);
            }

            mesh.color = vec3(shape.material.diffuse[0],
                shape.material.diffuse[1],
                shape.material.diffuse[2]);
            mesh.texname = shape.material.name;//diffuse_texname;
            device_mesh_t out;
            uploadMesh(mesh, out);
            m_meshes.push_back(out);
            f = f + process;
        }
    }
}

void GLApp::initQuad()
{
    vertex2_t verts[] = { { vec3(-1, 1, 0), vec2(0, 1) },
    { vec3(-1, -1, 0), vec2(0, 0) },
    { vec3(1, -1, 0), vec2(1, 0) },
    { vec3(1, 1, 0), vec2(1, 1) } };

    unsigned short indices[] = { 0, 1, 2, 0, 2, 3 };

    //Allocate vertex array
    //Vertex arrays encapsulate a set of generic vertex attributes and the buffers they are bound too
    //Different vertex array per mesh.
    glGenVertexArrays(1, &(m_Quad.vertex_array));
    glBindVertexArray(m_Quad.vertex_array);

    //Allocate vbos for data
    glGenBuffers(1, &(m_Quad.vbo_data));
    glGenBuffers(1, &(m_Quad.vbo_indices));

    //Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, m_Quad.vbo_data);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    //Use of strided data, Array of Structures instead of Structures of Arrays
    glVertexAttribPointer(quad_attributes::POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(vertex2_t), 0);
    glVertexAttribPointer(quad_attributes::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(vertex2_t), (void*)sizeof(vec3));
    glEnableVertexAttribArray(quad_attributes::POSITION);
    glEnableVertexAttribArray(quad_attributes::TEXCOORD);

    //indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Quad.vbo_indices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLushort), indices, GL_STATIC_DRAW);
    m_Quad.num_indices = 6;

    //Unplug Vertex Array
    glBindVertexArray(0);
}

void GLApp::InitShader()
{
    const char * pass_vert = "../res/shaders/pass.vert";
    const char * shade_vert = "../res/shaders/shade.vert";
    const char * post_vert = "../res/shaders/post.vert";

    const char * pass_frag = "../res/shaders/pass.frag";
    const char * diagnostic_frag = "../res/shaders/diagnostic.frag";
    const char * ambient_frag = "../res/shaders/ambient.frag";
    const char * point_frag = "../res/shaders/point.frag";
    const char * post_frag = "../res/shaders/post.frag";

    Utility::shaders_t shaders; 
    
    shaders = Utility::loadShaders(pass_vert, pass_frag);
    m_passProg = glCreateProgram();
    glBindAttribLocation(m_passProg, mesh_attributes::POSITION, "Position");
    glBindAttribLocation(m_passProg, mesh_attributes::NORMAL, "Normal");
    glBindAttribLocation(m_passProg, mesh_attributes::TEXCOORD, "Texcoord");
    Utility::attachAndLinkProgram(m_passProg, shaders);

    shaders = Utility::loadShaders(shade_vert, diagnostic_frag);
    m_diagnosticProg = glCreateProgram();
    glBindAttribLocation(m_diagnosticProg, quad_attributes::POSITION, "Position");
    glBindAttribLocation(m_diagnosticProg, quad_attributes::TEXCOORD, "Texcoord");
    Utility::attachAndLinkProgram(m_diagnosticProg, shaders);

    shaders = Utility::loadShaders(shade_vert, ambient_frag);
    m_ambientProg = glCreateProgram();
    glBindAttribLocation(m_ambientProg, quad_attributes::POSITION, "Position");
    glBindAttribLocation(m_ambientProg, quad_attributes::TEXCOORD, "Texcoord");
    Utility::attachAndLinkProgram(m_ambientProg, shaders);

    shaders = Utility::loadShaders(shade_vert, point_frag);
    m_pointProg = glCreateProgram();
    glBindAttribLocation(m_pointProg, quad_attributes::POSITION, "Position");
    glBindAttribLocation(m_pointProg, quad_attributes::TEXCOORD, "Texcoord");
    Utility::attachAndLinkProgram(m_pointProg, shaders);

    shaders = Utility::loadShaders(post_vert, post_frag);
    m_postProg = glCreateProgram();
    glBindAttribLocation(m_postProg, quad_attributes::POSITION, "Position");
    glBindAttribLocation(m_postProg, quad_attributes::TEXCOORD, "Texcoord");
    Utility::attachAndLinkProgram(m_postProg, shaders);
}

void GLApp::initNoise() 
{
    const char * rand_norm_png = "../res/random_normal.png";
    const char * rand_png = "../res/random.png";

    m_randomNormalTexture = (unsigned int)SOIL_load_OGL_texture(rand_norm_png, 0, 0, 0);
    glBindTexture(GL_TEXTURE_2D, m_randomNormalTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_randomScalarTexture = (unsigned int)SOIL_load_OGL_texture(rand_png, 0, 0, 0);
    glBindTexture(GL_TEXTURE_2D, m_randomScalarTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLApp::initFBO()
{
    GLenum FBOstatus;

    glActiveTexture(GL_TEXTURE9);

    glGenTextures(1, &m_depthTexture);
    glGenTextures(1, &m_normalTexture);
    glGenTextures(1, &m_positionTexture);
    glGenTextures(1, &m_colorTexture);
    glGenTextures(1, &m_glowmaskTexture);

    //Set up depth FBO
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    //Set up normal FBO
    glBindTexture(GL_TEXTURE_2D, m_normalTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);

    //Set up position FBO
    glBindTexture(GL_TEXTURE_2D, m_positionTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);

    //Set up color FBO
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);

    //Set up glowmap FBO
    glBindTexture(GL_TEXTURE_2D, m_glowmaskTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);
    glGenerateMipmap(GL_TEXTURE_2D);

    // creatwwe a framebuffer object
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Instruct openGL that we won't bind a color texture with the currently bound FBO
    glReadBuffer(GL_NONE);
    GLint normal_loc = glGetFragDataLocation(m_passProg, "out_Normal");
    GLint position_loc = glGetFragDataLocation(m_passProg, "out_Position");
    GLint color_loc = glGetFragDataLocation(m_passProg, "out_Color");
    GLint glowmask_loc = glGetFragDataLocation(m_passProg, "out_GlowMask");

    GLenum draws[4];
    draws[normal_loc] = GL_COLOR_ATTACHMENT0;
    draws[position_loc] = GL_COLOR_ATTACHMENT1;
    draws[color_loc] = GL_COLOR_ATTACHMENT2;
    draws[glowmask_loc] = GL_COLOR_ATTACHMENT3;
    glDrawBuffers(4, draws);

    // attach the texture to FBO depth attachment point
    int test = GL_COLOR_ATTACHMENT0;
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthTexture, 0);
    glBindTexture(GL_TEXTURE_2D, m_normalTexture);
    glFramebufferTexture(GL_FRAMEBUFFER, draws[normal_loc], m_normalTexture, 0);
    glBindTexture(GL_TEXTURE_2D, m_positionTexture);
    glFramebufferTexture(GL_FRAMEBUFFER, draws[position_loc], m_positionTexture, 0);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glFramebufferTexture(GL_FRAMEBUFFER, draws[color_loc], m_colorTexture, 0);
    glBindTexture(GL_TEXTURE_2D, m_glowmaskTexture);
    glFramebufferTexture(GL_FRAMEBUFFER, draws[glowmask_loc], m_glowmaskTexture, 0);

    // check FBO status
    FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (FBOstatus == GL_FRAMEBUFFER_COMPLETE)
        m_FBO.push_back(fbo);

    //Post Processing buffer!
    glActiveTexture(GL_TEXTURE9);

    glGenTextures(1, &m_postTexture);
    //Set up post FBO
    glBindTexture(GL_TEXTURE_2D, m_postTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, 0);

    // create a framebuffer object
    fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Instruct openGL that we won't bind a color texture with the currently bound FBO
    glReadBuffer(GL_BACK);
    color_loc = glGetFragDataLocation(m_ambientProg, "out_Color");
    GLenum draw[1];
    draw[color_loc] = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, draw);

    // attach the texture to FBO depth attachment point
    test = GL_COLOR_ATTACHMENT0;
    glBindTexture(GL_TEXTURE_2D, m_postTexture);
    glFramebufferTexture(GL_FRAMEBUFFER, draw[color_loc], m_postTexture, 0);

    // check FBO status
    FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (FBOstatus == GL_FRAMEBUFFER_COMPLETE)
        m_FBO.push_back(fbo);

    // switch back to window-system-provided framebuffer
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GLApp::bindFBO(uint32_t buf) 
{
    assert(buf < m_FBO.size());
    glBindTexture(GL_TEXTURE_2D, 0); //Bad mojo to unbind the framebuffer using the texture
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO[buf]);
    glClear(GL_DEPTH_BUFFER_BIT);
    //glColorMask(false,false,false,false);
    glEnable(GL_DEPTH_TEST);
}

void GLApp::setTextures() 
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //glColorMask(true,true,true,true);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLApp::setupQuad(GLuint prog)
{
    glUseProgram(prog);

    mat4 persp = glm::perspective(45.0f, (float)m_width / (float)m_height, m_nearPlane, m_farPlane);
    vec4 test(-2, 0, 10, 1);
    vec4 testp = persp * test;
    vec4 testh = testp / testp.w;
    vec2 coords = vec2(testh.x, testh.y) / 2.0f + 0.5f;
    glUniform1i(glGetUniformLocation(prog, "u_ScreenHeight"), m_height);
    glUniform1i(glGetUniformLocation(prog, "u_ScreenWidth"), m_width);
    glUniform1f(glGetUniformLocation(prog, "u_Far"), m_farPlane);
    glUniform1f(glGetUniformLocation(prog, "u_Near"), m_nearPlane);
    glUniform1i(glGetUniformLocation(prog, "u_DisplayType"), m_displayType);
    glUniformMatrix4fv(glGetUniformLocation(prog, "u_Persp"), 1, GL_FALSE, &persp[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glUniform1i(glGetUniformLocation(prog, "u_Depthtex"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_normalTexture);
    glUniform1i(glGetUniformLocation(prog, "u_Normaltex"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_positionTexture);
    glUniform1i(glGetUniformLocation(prog, "u_Positiontex"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glUniform1i(glGetUniformLocation(prog, "u_Colortex"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_randomNormalTexture);
    glUniform1i(glGetUniformLocation(prog, "u_RandomNormaltex"), 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_randomScalarTexture);
    glUniform1i(glGetUniformLocation(prog, "u_RandomScalartex"), 5);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, m_glowmaskTexture);
    glUniform1i(glGetUniformLocation(prog, "u_GlowMask"), 6);
}


void GLApp::drawMeshes()
{
    m_farPlane = 100.0f;
    m_nearPlane = 0.1f;

    glUseProgram(m_passProg);

    mat4 view = m_cam->get_view();
    mat4 persp = glm::perspective(45.0f, (float)m_width / (float)m_height, m_nearPlane, m_farPlane);
    mat4 inverse_transposed = glm::transpose(glm::inverse(view*m_world));

    glUniform1f(glGetUniformLocation(m_passProg, "u_Far"), m_farPlane);
    glUniformMatrix4fv(glGetUniformLocation(m_passProg, "u_Model"), 1, GL_FALSE, &m_world[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_passProg, "u_View"), 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_passProg, "u_Persp"), 1, GL_FALSE, &persp[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(m_passProg, "u_InvTrans"), 1, GL_FALSE, &inverse_transposed[0][0]);

    for (uint32_t i = 0; i < m_meshes.size(); i++)
    {
        glUniform3fv(glGetUniformLocation(m_passProg, "u_Color"), 1, &(m_meshes[i].color[0]));
        glBindVertexArray(m_meshes[i].vertex_array);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshes[i].vbo_indices);

        float glowmask = 0.0f;
        if (m_meshes[i].texname == "light")
            glowmask = 1.0f;
        glUniform1f(glGetUniformLocation(m_passProg, "glowmask"), glowmask);
        glDrawElements(GL_TRIANGLES, m_meshes[i].num_indices, GL_UNSIGNED_SHORT, 0);
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void GLApp::drawQuad()
{
    glBindVertexArray(m_Quad.vertex_array);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Quad.vbo_indices);

    glDrawElements(GL_TRIANGLES, m_Quad.num_indices, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);
}

void GLApp::drawLight(vec3 pos, float strength, mat4 sc, mat4 vp)
{
    float radius = strength;
    vec4 light = m_cam->get_view() * vec4(pos, 1.0);
    if (light.z > m_nearPlane)
    {
        return;
    }
    light.w = radius;
    glUniform4fv(glGetUniformLocation(m_pointProg, "u_Light"), 1, &(light[0]));
    glUniform1f(glGetUniformLocation(m_pointProg, "u_LightIl"), strength);

    vec4 left = vp * vec4(pos + radius*m_cam->start_left, 1.0);
    vec4 up = vp * vec4(pos + radius*m_cam->up, 1.0);
    vec4 center = vp * vec4(pos, 1.0);

    left = sc * left;
    up = sc * up;
    center = sc * center;

    left /= left.w;
    up /= up.w;
    center /= center.w;

    float hw = glm::distance(left, center);
    float hh = glm::distance(up, center);

    float r = (hh > hw) ? hh : hw;

    float x = center.x - r;
    float y = center.y - r;

    glScissor(x, y, 2 * r, 2 * r);
    drawQuad();
}

void GLApp::display()
{
    m_cam->CalculateView();

    // Stage 1 -- RENDER TO G-BUFFER
    bindFBO(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    drawMeshes();
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, m_glowmaskTexture);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Stage 2 -- RENDER TO P-BUFFER
    setTextures();
    bindFBO(1);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_ONE, GL_ONE);
    glClear(GL_COLOR_BUFFER_BIT);
    if (m_displayType == DISPLAY_LIGHTS || m_displayType == DISPLAY_TOTAL)
    {
        setupQuad(m_pointProg);
        if (m_scissorEnabled) 
            glEnable(GL_SCISSOR_TEST);
        mat4 vp = glm::perspective(45.0f, (float)m_width / (float)m_height, m_nearPlane, m_farPlane) * m_cam->get_view();
        mat4 sc = mat4(m_width, 0.0, 0.0, 0.0,
            0.0, m_height, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0) *
            mat4(0.5, 0.0, 0.0, 0.0,
            0.0, 0.5, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.5, 0.5, 0.0, 1.0);

        glm::vec3 yellow = glm::vec3(1, 1, 0);
        glm::vec3 orange = glm::vec3(0.89, 0.44, 0.1);
        glm::vec3 red = glm::vec3(1, 0, 0);
        glm::vec3 blue = glm::vec3(0, 0, 1);

        glUniform1i(glGetUniformLocation(m_pointProg, "u_toonOn"), m_toonEnabled);
        glUniform3fv(glGetUniformLocation(m_pointProg, "u_LightCol"), 1, &(yellow[0]));
        drawLight(vec3(5.4, -0.5, 3.0), 1.0, sc, vp);
        drawLight(vec3(0.2, -0.5, 3.0), 1.0, sc, vp);
        glUniform3fv(glGetUniformLocation(m_pointProg, "u_LightCol"), 1, &(orange[0]));
        drawLight(vec3(5.4, -2.5, 3.0), 1.0, sc, vp);
        drawLight(vec3(0.2, -2.5, 3.0), 1.0, sc, vp);
        glUniform3fv(glGetUniformLocation(m_pointProg, "u_LightCol"), 1, &(yellow[0]));
        drawLight(vec3(5.4, -4.5, 3.0), 1.0, sc, vp);
        drawLight(vec3(0.2, -4.5, 3.0), 1.0, sc, vp);

        glUniform3fv(glGetUniformLocation(m_pointProg, "u_LightCol"), 1, &(red[0]));
        drawLight(vec3(2.5, -1.2, 0.5), 2.5, sc, vp);

        glUniform3fv(glGetUniformLocation(m_pointProg, "u_LightCol"), 1, &(blue[0]));
        drawLight(vec3(2.5, -5.0, 4.2), 2.5, sc, vp);

        glDisable(GL_SCISSOR_TEST);
        vec4 dir_light(0.1, 1.0, 1.0, 0.0);
        dir_light = m_cam->get_view() * dir_light;
        dir_light = glm::normalize(dir_light);
        dir_light.w = 0.3f;
        float strength = 0.09f;

        setupQuad(m_ambientProg);
        glUniform1i(glGetUniformLocation(m_ambientProg, "u_toonOn"), m_toonEnabled);
        glUniform4fv(glGetUniformLocation(m_ambientProg, "u_Light"), 1, &(dir_light[0]));
        glUniform1f(glGetUniformLocation(m_ambientProg, "u_LightIl"), strength);
        drawQuad();
    }
    else
    {
        setupQuad(m_diagnosticProg);
        drawQuad();
    }
    glDisable(GL_BLEND);

    //Stage 3 -- RENDER TO SCREEN
    setTextures();
    glUseProgram(m_postProg);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_postTexture);
    glUniform1i(glGetUniformLocation(m_postProg, "u_Posttex"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_glowmaskTexture);
    glUniform1i(glGetUniformLocation(m_postProg, "u_GlowMask"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_normalTexture);
    glUniform1i(glGetUniformLocation(m_postProg, "u_normalTex"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_positionTexture);
    glUniform1i(glGetUniformLocation(m_postProg, "u_positionTex"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m_randomNormalTexture);
    glUniform1i(glGetUniformLocation(m_postProg, "u_RandomNormaltex"), 4);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_randomScalarTexture);
    glUniform1i(glGetUniformLocation(m_postProg, "u_RandomScalartex"), 5);

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glUniform1i(glGetUniformLocation(m_postProg, "u_depthTex"), 6);

    glUniform1i(glGetUniformLocation(m_postProg, "u_ScreenHeight"), m_height);
    glUniform1i(glGetUniformLocation(m_postProg, "u_ScreenWidth"), m_width);
    glUniform1f(glGetUniformLocation(m_postProg, "u_InvScrHeight"), m_invHeight);
    glUniform1f(glGetUniformLocation(m_postProg, "u_InvScrWidth"), m_invWidth);
    glUniform1f(glGetUniformLocation(m_postProg, "u_mouseTexX"), mouse_dof_x*m_invWidth);
    glUniform1f(glGetUniformLocation(m_postProg, "u_mouseTexY"), abs(static_cast<int32_t>(m_height) - mouse_dof_y)*m_invHeight);
    glUniform1f(glGetUniformLocation(m_postProg, "u_lenQuant"), 0.0025f);
    glUniform1f(glGetUniformLocation(m_postProg, "u_Far"), m_farPlane);
    glUniform1f(glGetUniformLocation(m_postProg, "u_Near"), m_nearPlane);
    glUniform1i(glGetUniformLocation(m_postProg, "u_BloomOn"), m_bloomEnabled);
    glUniform1i(glGetUniformLocation(m_postProg, "u_toonOn"), m_toonEnabled);
    glUniform1i(glGetUniformLocation(m_postProg, "u_DOFOn"), m_DOFEnabled);
    glUniform1i(glGetUniformLocation(m_postProg, "u_DOFDebug"), m_DOFDebug);
    drawQuad();

    glEnable(GL_DEPTH_TEST);
    //    updateTitle();
}

void GLApp::reshape(int w, int h)
{
    m_width = w;
    m_height = h;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    if (m_depthTexture)
        glDeleteTextures(1, &m_depthTexture);
    if (m_normalTexture)
        glDeleteTextures(1, &m_normalTexture);
    if (m_positionTexture)
        glDeleteTextures(1, &m_positionTexture);
    if (m_colorTexture)
        glDeleteTextures(1, &m_colorTexture);
    if (m_postTexture)
        glDeleteTextures(1, &m_postTexture);

    for (GLuint& fbo : m_FBO)
        glDeleteFramebuffers(1, &fbo);

    initFBO();
}

void GLApp::AdjustCamera(float xAdjustment, float yAdjustment, float zAdjustment)
{ 
    if (m_cam)
        m_cam->adjust(0.0f, 0.0f, 0.0f, xAdjustment, yAdjustment, zAdjustment); 
}

void GLApp::RotateCamera(float xAngle, float yAngle)
{
    if (m_cam)
        m_cam->adjust(xAngle, yAngle, 0.0f, 0.0f, 0.0f, 0.0f);
}

int32_t GLApp::init(std::vector<tinyobj::shape_t>& shapes)
{
    if (!glfwInit())
        return 0;

    /* Create a windowed mode window and its OpenGL context */
    m_glfwWindow = glfwCreateWindow(m_width, m_height, m_windowTitle.c_str(), NULL, NULL);
    if (!m_glfwWindow)
    {
        glfwTerminate();
        return 0;
    }
    glfwMakeContextCurrent(m_glfwWindow);

    glfwSetInputMode(m_glfwWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(m_glfwWindow, EventHandler::OnKeyPress);
    glfwSetMouseButtonCallback(m_glfwWindow, EventHandler::OnMouseClick);
    glfwSetCursorPosCallback(m_glfwWindow, EventHandler::OnMouseMove);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        return 0;
    }

    initNoise();
    InitShader();
    initFBO();
    initMesh(shapes);
    initQuad();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    return 1;
}

int32_t GLApp::Run()
{
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(m_glfwWindow))
    {
        /* Render here */
        display();

        /* Swap front and back buffers */
        glfwSwapBuffers(m_glfwWindow);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}