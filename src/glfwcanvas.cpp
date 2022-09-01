#include <functional>

#include "canvas.h"
#include "shader/circle.h"
#include "shader/thickline.h"
#include "shader/triangle.h"

#define GL_CALL(func)                                                     \
  func;                                                                   \
  {                                                                       \
    uint32_t error_code = 0;                                              \
    while ((error_code = glGetError())) {                                 \
      std::cerr << WHERE << " OpenGl error: 0x" << std::hex << error_code \
                << "\n";                                                  \
      DEBUG_BREAK                                                         \
    }                                                                     \
  }

namespace Canvas {

void GLFWCanvas::set_event_callbacks() {
  glfwSetWindowCloseCallback(window, [](GLFWwindow* window) {
    static_cast<GLFWCanvas*>(glfwGetWindowUserPointer(window))->stop();
  });

  glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode,
                                int action, int mods) {
    auto self = static_cast<GLFWCanvas*>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS) {
      self->event_queue.push_back(KeydownEvent());
    } else if (action == GLFW_RELEASE) {
      self->event_queue.push_front(KeyupEvent());
    } else {
      self->event_queue.push_front({});
    }
  });

  glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y) {
    auto self = static_cast<GLFWCanvas*>(glfwGetWindowUserPointer(window));
    self->event_queue.push_front(
        MouseMoveEvent{.x = (uint32_t)x, .y = (uint32_t)y});
  });

  glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button,
                                        int action, int mods) {
    auto self = static_cast<GLFWCanvas*>(glfwGetWindowUserPointer(window));
    if (action == GLFW_PRESS)
      self->event_queue.push_front(MouseDownEvent{
          .button = (button == GLFW_MOUSE_BUTTON_LEFT) ? MouseButton::LEFT
                                                       : MouseButton::RIGHT});
  });

  glfwSetWindowSizeCallback(
      window, [](GLFWwindow* window, int width, int height) {
        auto self = static_cast<GLFWCanvas*>(glfwGetWindowUserPointer(window));
        GL_CALL(glViewport(0, 0, width, height));
        self->event_queue.push_front(WindowResizeEvent{
            .new_width = (uint32_t)width,
            .new_height = (uint32_t)height,
        });
      });
}

static uint32_t compile_shader(const char* src, uint32_t shader_type) {
  uint32_t shader_id = 0;
  GL_CALL(shader_id = glCreateShader(shader_type));
  GL_CALL(glShaderSource(shader_id, 1, &src, nullptr));
  GL_CALL(glCompileShader(shader_id));

  int compile_status = 0;
  GL_CALL(glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_status));
  if (!compile_status) {
    int log_length = 0;
    GL_CALL(glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length));
    std::string msg;
    msg.resize(log_length);
    GL_CALL(glGetShaderInfoLog(shader_id, log_length, &log_length, msg.data()));
    std::cerr << WHERE << " Linking error: " << msg << "\n";
  }

  return shader_id;
}

uint32_t GLFWCanvas::load_shader(const char* vert_src, const char* geom_src,
                                 const char* frag_src) {
  std::optional<int> vert_id, geom_id, frag_id;

  uint32_t program_id = 0;
  GL_CALL(program_id = glCreateProgram());
  if (vert_src) {
    vert_id = compile_shader(vert_src, GL_VERTEX_SHADER);
    GL_CALL(glAttachShader(program_id, vert_id.value()));
    GL_CALL(glDeleteShader(vert_id.value()));
  }
  if (frag_src) {
    frag_id = compile_shader(frag_src, GL_FRAGMENT_SHADER);
    GL_CALL(glAttachShader(program_id, frag_id.value()));
    GL_CALL(glDeleteShader(frag_id.value()));
  }
  if (geom_src) {
    geom_id = compile_shader(geom_src, GL_GEOMETRY_SHADER);
    GL_CALL(glAttachShader(program_id, geom_id.value()));
    GL_CALL(glDeleteShader(geom_id.value()));
  }

  GL_CALL(glLinkProgram(program_id));
  GL_CALL(glValidateProgram(program_id));
  int status = 0;
  GL_CALL(glGetProgramiv(program_id, GL_LINK_STATUS, &status));
  if (status != GL_TRUE) {
    int log_length = 0;
    GL_CALL(glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length));
    std::string msg;
    msg.resize(log_length);
    GL_CALL(
        glGetProgramInfoLog(program_id, log_length, &log_length, msg.data()));
    std::cerr << WHERE << " Linking error: " << msg << "\n";
  }

  GL_CALL(glUseProgram(program_id));

  return program_id;
}

void GLFWCanvas::error_callback(int error, const char* error_message) {
  std::cerr << "GLFW Error 0x" << std::hex << error << ", " << error_message
            << "\n";
}

void GLFWCanvas::set_viewport(Viewport new_viewport) {
  float near = 0.0f, far = 1.0f;
  WindowCanvas::set_viewport(new_viewport);

  mvp[0] = 2.0f / (viewport.right - viewport.left);
  mvp[1] = 0.0f;
  mvp[2] = 0.0f;
  mvp[3] = 0.0f;

  mvp[4] = 0.0f;
  mvp[5] = 2.0f / (viewport.top - viewport.bottom);
  mvp[6] = 0.0f;
  mvp[7] = 0.0f;

  mvp[8] = 0.0f;
  mvp[9] = 0.0f;
  mvp[10] = 2.0f / (near - far);
  mvp[11] = 0.0f;

  mvp[12] = (viewport.right + viewport.left) / (viewport.left - viewport.right);
  mvp[13] = (viewport.top + viewport.bottom) / (viewport.bottom - viewport.top);
  mvp[14] = (far + near) / (near - far);
  mvp[15] = 1.0f;

  GL_CALL(glUseProgram(shaders[TRIANGLE]));
  GL_CALL(glUniformMatrix4fv(umvps[TRIANGLE], 1, GL_FALSE, mvp.data()));
  GL_CALL(glUseProgram(shaders[CIRCLE]));
  GL_CALL(glUniformMatrix4fv(umvps[CIRCLE], 1, GL_FALSE, mvp.data()));
  GL_CALL(glUseProgram(shaders[THICK_LINE]));
  GL_CALL(glUniformMatrix4fv(umvps[THICK_LINE], 1, GL_FALSE, mvp.data()));
}

GLFWCanvas::GLFWCanvas(uint32_t width, uint32_t height,
                       const std::string& title,
                       std::shared_ptr<WindowHandler>&& handler,
                       Viewport viewport)
    : WindowCanvas(std::move(handler), viewport), width(width), height(height) {
  ASSERT(glfwInit(), == true);
  glfwSetErrorCallback(GLFWCanvas::error_callback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  ASSERT((window =
              glfwCreateWindow(width, height, title.data(), nullptr, nullptr)),
         != nullptr);

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  ASSERT(gl3wInit(), == false);
  GL_CALL(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
  GL_CALL(glEnable(GL_BLEND));
  GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

  glfwSetWindowUserPointer(window, this);
  set_event_callbacks();

  GL_CALL(glEnable(GL_MULTISAMPLE));

  GL_CALL(glGenVertexArrays(4, vaos.data()));
  GL_CALL(glGenBuffers(4, vbos.data()));

  GL_CALL(glBindVertexArray(vaos[TRIANGLE]));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbos[TRIANGLE]));
  GL_CALL(glEnableVertexAttribArray(0));
  GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                                nullptr));
  shaders[TRIANGLE] = load_shader(triangle_shader_vert_source, nullptr,
                                  triangle_shader_frag_source);
  GL_CALL(
      ASSERT(umvps[TRIANGLE] = glGetUniformLocation(shaders[TRIANGLE], "uMVP"),
             != -1));
  GL_CALL(ASSERT(
      ucolors[TRIANGLE] = glGetUniformLocation(shaders[TRIANGLE], "uColor"),
      != -1));

  GL_CALL(glBindVertexArray(vaos[CIRCLE]));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbos[CIRCLE]));
  GL_CALL(glEnableVertexAttribArray(0));
  GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                                nullptr));
  GL_CALL(glEnableVertexAttribArray(1));
  GL_CALL(glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                                (void*)(2 * sizeof(float))));
  shaders[CIRCLE] =
      load_shader(circle_shader_vert_source, circle_shader_geom_source,
                  circle_shader_frag_source);
  GL_CALL(ASSERT(umvps[CIRCLE] = glGetUniformLocation(shaders[CIRCLE], "uMVP"),
                 != -1));
  GL_CALL(
      ASSERT(ucolors[CIRCLE] = glGetUniformLocation(shaders[CIRCLE], "uColor"),
             != -1));

  GL_CALL(glBindVertexArray(vaos[THICK_LINE]));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbos[THICK_LINE]));
  GL_CALL(glEnableVertexAttribArray(0));
  GL_CALL(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                                nullptr));
  GL_CALL(glEnableVertexAttribArray(1));
  GL_CALL(glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                                (void*)(2 * sizeof(float))));
  shaders[THICK_LINE] =
      load_shader(thick_line_shader_vert_source, thick_line_shader_geom_source,
                  thick_line_shader_frag_source);
  GL_CALL(ASSERT(
      umvps[THICK_LINE] = glGetUniformLocation(shaders[THICK_LINE], "uMVP"),
      != -1));
  GL_CALL(ASSERT(
      ucolors[THICK_LINE] = glGetUniformLocation(shaders[THICK_LINE], "uColor"),
      != -1));

  set_viewport(viewport);
  GL_CALL(glViewport(0, 0, width, height));
}

GLFWCanvas::~GLFWCanvas() {
  GL_CALL(glDeleteProgram(shaders[TRIANGLE]));
  GL_CALL(glDeleteVertexArrays(4, vaos.data()));
  GL_CALL(glDeleteBuffers(4, vbos.data()));

  glfwDestroyWindow(window);
  glfwTerminate();
}

void GLFWCanvas::display() {
  while (!glfwWindowShouldClose(window) && !has_quit()) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    update();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

std::optional<Event> GLFWCanvas::next_event() {
  if (event_queue.empty()) return {};
  auto next = event_queue.back();
  event_queue.pop_back();
  return next;
}

void GLFWCanvas::draw_primitive(const Line& l) {
  if (l.thickness == 0.0f) {
    std::array<float, 4> pts = {l.start.x, l.start.y, l.end.x, l.end.y};

    GL_CALL(glUseProgram(shaders[TRIANGLE]));
    GL_CALL(glUniform4f(ucolors[TRIANGLE], l.color.r, l.color.g, l.color.b,
                        l.color.a));
    GL_CALL(glBindVertexArray(vaos[TRIANGLE]));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbos[TRIANGLE]));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(pts[0]),
                         pts.data(), GL_DYNAMIC_DRAW));
    GL_CALL(glDrawArrays(GL_LINES, 0, pts.size() / 2));
  } else {
    std::array<float, 6> pts = {l.start.x, l.start.y, l.thickness,
                                l.end.x,   l.end.y,   l.thickness};

    GL_CALL(glUseProgram(shaders[THICK_LINE]));
    GL_CALL(glUniform4f(ucolors[THICK_LINE], l.color.r, l.color.g, l.color.b,
                        l.color.a));
    GL_CALL(glBindVertexArray(vaos[THICK_LINE]));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbos[THICK_LINE]));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(pts[0]),
                         pts.data(), GL_DYNAMIC_DRAW));
    GL_CALL(glDrawArrays(GL_LINES, 0, pts.size() / 3));
  }
}

void GLFWCanvas::draw_primitive(const Circle& c) {
  std::array<float, 3> pts = {c.origin.x, c.origin.y, c.radius};

  GL_CALL(glUseProgram(shaders[CIRCLE]));
  GL_CALL(
      glUniform4f(ucolors[CIRCLE], c.color.r, c.color.g, c.color.b, c.color.a));
  GL_CALL(glBindVertexArray(vaos[CIRCLE]));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbos[CIRCLE]));
  GL_CALL(glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(pts[0]), pts.data(),
                       GL_DYNAMIC_DRAW));
  GL_CALL(glDrawArrays(GL_POINTS, 0, pts.size() / 3));
}

void GLFWCanvas::draw_primitive(const Triangle& p) {
  std::array<float, 6> pts = {
      p.points[0].x, p.points[0].y, p.points[1].x,
      p.points[1].y, p.points[2].x, p.points[2].y,
  };

  GL_CALL(glUseProgram(shaders[TRIANGLE]));
  GL_CALL(glUniform4f(ucolors[TRIANGLE], p.color.r, p.color.g, p.color.b,
                      p.color.a));
  GL_CALL(glBindVertexArray(vaos[TRIANGLE]));
  GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vbos[TRIANGLE]));
  GL_CALL(glBufferData(GL_ARRAY_BUFFER, pts.size() * sizeof(pts[0]), pts.data(),
                       GL_DYNAMIC_DRAW));
  GL_CALL(glDrawArrays(GL_TRIANGLES, 0, pts.size() / 2));
}
}  // namespace Canvas
