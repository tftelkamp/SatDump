#include <GLFW/glfw3.h>
#include "core/style.h"
#include "backend.h"

extern GLFWwindow* window;
extern bool fallback_gl;

float funcDeviceScale()
{
    float display_scale;
#if GLFW_VERSION_MAJOR > 3 || (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 3)
    glfwGetWindowContentScale(window, &display_scale, nullptr);
    display_scale /= style::macos_framebuffer_scale();
#else
    display_scale = 1.0f;
#endif
    return display_scale;
}

void funcRebuildFonts()
{
#ifndef IMGUI_IMPL_OPENGL_ES2
    if (fallback_gl)
    {
        ImGui_ImplOpenGL2_DestroyFontsTexture();
        ImGui_ImplOpenGL2_CreateFontsTexture();
    }
    else
#endif
    {
        ImGui_ImplOpenGL3_DestroyFontsTexture();
        ImGui_ImplOpenGL3_CreateFontsTexture();
    }
}

void funcSetMousePos(int x, int y)
{
    glfwSetCursorPos(window, x, y);
    ImGui_ImplGlfw_CursorPosCallback(window, x, y);
}

std::pair<int, int> funcBeginFrame()
{
    // Start the Dear ImGui frame
#ifndef IMGUI_IMPL_OPENGL_ES2
    if (fallback_gl)
        ImGui_ImplOpenGL2_NewFrame();
    else
#endif
        ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    int wwidth, wheight;
    glfwGetWindowSize(window, &wwidth, &wheight);
    return {wwidth, wheight};
}

void funcEndFrame()
{
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(style::theme.frame_bg.Value.x, style::theme.frame_bg.Value.y, style::theme.frame_bg.Value.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

#ifndef IMGUI_IMPL_OPENGL_ES2
    if (fallback_gl)
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    else
#endif
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void funcSetIcon(uint8_t *image, int w, int h)
{
#ifndef _WIN32
    GLFWimage img;
    img.pixels = image;
    img.width = w;
    img.height = h;
    glfwSetWindowIcon(window, 1, &img);
#endif
}

void bindBackendFunctions()
{
    backend::device_scale = funcDeviceScale();

    backend::rebuildFonts = funcRebuildFonts;
    backend::setMousePos = funcSetMousePos;
    backend::beginFrame = funcBeginFrame;
    backend::endFrame = funcEndFrame;
    backend::setIcon = funcSetIcon;
}
