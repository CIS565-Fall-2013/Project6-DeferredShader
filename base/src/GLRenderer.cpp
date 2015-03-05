#include "GLRenderer.h"
#include "Utility.h"
#include "gl/glew.h"
#include "SOIL/SOIL.h"

GLRenderer::GLRenderer(uint32_t width, uint32_t height)
    : m_width(width),
    m_height(height),
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
    m_postProg(0)
{
    m_invWidth = 1.0f / m_width;
    m_invHeight = 1.0f / m_height;
}

void GLRenderer::InitShaders()
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

void GLRenderer::InitNoise()
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

void GLRenderer::InitFBO()
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

void GLRenderer::InitQuad()
{
    vertex2_t verts[] = 
    { 
        { glm::vec3(-1, 1, 0), glm::vec2(0, 1) },
        { glm::vec3(-1, -1, 0), glm::vec2(0, 0) },
        { glm::vec3(1, -1, 0), glm::vec2(1, 0) },
        { glm::vec3(1, 1, 0), glm::vec2(1, 1) }
    };

    uint16_t indices[] = { 0, 1, 2, 0, 2, 3 };

    //Allocate vertex array
    //Vertex arrays encapsulate a set of generic vertex attributes and the buffers they are bound too
    //Different vertex array per mesh.
    glGenVertexArrays(1, &(m_QuadGeometry.vertex_array));
    glBindVertexArray(m_QuadGeometry.vertex_array);

    //Allocate vbos for data
    uint32_t vbo_data;
    glGenBuffers(1, &(vbo_data));
    glGenBuffers(1, &(m_QuadGeometry.index_buffer));

    //Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vbo_data);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    //Use of strided data, Array of Structures instead of Structures of Arrays
    glVertexAttribPointer(quad_attributes::POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(vertex2_t), 0);
    glVertexAttribPointer(quad_attributes::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(vertex2_t), (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(quad_attributes::POSITION);
    glEnableVertexAttribArray(quad_attributes::TEXCOORD);

    //indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadGeometry.index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLushort), indices, GL_STATIC_DRAW);
    m_QuadGeometry.num_indices = 6;

    //Unplug Vertex Array
    glBindVertexArray(0);
}

void GLRenderer::MakeDrawableGeometry(const Geometry& model, DrawableGeometry& out)
{
    //Allocate vertex array
    //Vertex arrays encapsulate a set of generic vertex 
    //attributes and the buffers they are bound to
    //Different vertex array per mesh.
    glGenVertexArrays(1, &(out.vertex_array));
    glBindVertexArray(out.vertex_array);

    //Allocate vbos for data
    uint32_t vbo_vertices, vbo_normals, vbo_texcoords;
    glGenBuffers(1, &(vbo_vertices));
    glGenBuffers(1, &(vbo_normals));
    glGenBuffers(1, &(vbo_texcoords));
    glGenBuffers(1, &(out.index_buffer));

    //Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
    glBufferData(GL_ARRAY_BUFFER, model.vertices.size()*sizeof(glm::vec3), &model.vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(mesh_attributes::POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(mesh_attributes::POSITION);

    //Upload normal data
    glBindBuffer(GL_ARRAY_BUFFER, vbo_normals);
    glBufferData(GL_ARRAY_BUFFER, model.normals.size()*sizeof(glm::vec3), &model.normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(mesh_attributes::NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(mesh_attributes::NORMAL);

    //Upload texture coord data
    glBindBuffer(GL_ARRAY_BUFFER, vbo_texcoords);
    glBufferData(GL_ARRAY_BUFFER, model.texcoords.size()*sizeof(glm::vec2), &model.texcoords[0], GL_STATIC_DRAW);
    glVertexAttribPointer(mesh_attributes::TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(mesh_attributes::TEXCOORD);

    //indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out.index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indices.size()*sizeof(GLushort), &model.indices[0], GL_STATIC_DRAW);
    out.num_indices = model.indices.size();

    //Unplug Vertex Array
    glBindVertexArray(0);

    out.texname = model.texname;
    out.color = model.color;
}

void GLRenderer::RenderQuad()
{
    glBindVertexArray(m_QuadGeometry.vertex_array);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_QuadGeometry.index_buffer);

    glDrawElements(GL_TRIANGLES, m_QuadGeometry.num_indices, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);
}

void GLRenderer::Render()
{

}