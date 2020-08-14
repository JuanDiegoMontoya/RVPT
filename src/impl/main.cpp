//
// Created by AregevDev on 23/04/2020.
//

#include <imgui.h>
#include <fmt/core.h>

#include "../rvpt/rvpt.h"

void setup_imgui(GLFWwindow* window)
{
    // Setup back-end capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |=
        ImGuiBackendFlags_HasMouseCursors;  // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;  // We can honor io.WantSetMousePos
    // requests (optional, rarely used)
    io.BackendPlatformName = "imgui_impl_glfw";

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    int w, h;
    int display_w, display_h;
    glfwGetWindowSize(window, &w, &h);
    glfwGetFramebufferSize(window, &display_w, &display_h);
    io.DisplaySize = ImVec2((float)w, (float)h);
    if (w > 0 && h > 0)
        io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);
}

void update_camera(Window& window, RVPT& rvpt)
{
    glm::vec3 movement{};
    double frameDelta = rvpt.time.since_last_frame();

    if (window.is_key_held(Window::KeyCode::KEY_LEFT_SHIFT)) frameDelta *= 5;
    if (window.is_key_held(Window::KeyCode::SPACE)) movement.y += 3.0f;
    if (window.is_key_held(Window::KeyCode::KEY_LEFT_CONTROL)) movement.y -= 3.0f;
    if (window.is_key_held(Window::KeyCode::KEY_W)) movement.z += 3.0f;
    if (window.is_key_held(Window::KeyCode::KEY_S)) movement.z -= 3.0f;
    if (window.is_key_held(Window::KeyCode::KEY_D)) movement.x += 3.0f;
    if (window.is_key_held(Window::KeyCode::KEY_A)) movement.x -= 3.0f;

    rvpt.scene_camera.move(static_cast<float>(frameDelta) * movement);

    glm::vec3 rotation{};
    float rot_speed = 0.3f;
    if (window.is_key_down(Window::KeyCode::KEY_RIGHT)) rotation.x = rot_speed;
    if (window.is_key_down(Window::KeyCode::KEY_LEFT)) rotation.x = -rot_speed;
    if (window.is_key_down(Window::KeyCode::KEY_UP)) rotation.y = -rot_speed;
    if (window.is_key_down(Window::KeyCode::KEY_DOWN)) rotation.y = rot_speed;
    rvpt.scene_camera.rotate(rotation);
}

int main()
{
    Window::Settings settings;
    settings.width = 1024;
    settings.height = 512;
    Window window(settings);

    RVPT rvpt(window);

    bool rvpt_init_ret = rvpt.initialize();
    if (!rvpt_init_ret)
    {
        fmt::print("failed to initialize RVPT\n");
        return 0;
    }
    setup_imgui(window.get_window_pointer());
    window.add_mouse_move_callback([&window, &rvpt](double x, double y) {
        if (window.is_mouse_locked_to_window())
        {
            rvpt.scene_camera.rotate(glm::vec3(x * 0.3f, -y * 0.3f, 0));
        }
    });

    window.add_mouse_click_callback([&window](Window::Mouse button, Window::Action action) {
        if (button == Window::Mouse::LEFT && action == Window::Action::RELEASE &&
            window.is_mouse_locked_to_window())
        {
            window.set_mouse_window_lock(false);
        }
        else if (button == Window::Mouse::LEFT && action == Window::Action::RELEASE &&
                 !window.is_mouse_locked_to_window())
        {
            if (!ImGui::GetIO().WantCaptureMouse)
            {
                window.set_mouse_window_lock(true);
            }
        }
    });
    while (!window.should_close())
    {
        window.poll_events();
        if (window.is_key_down(Window::KeyCode::KEY_ESCAPE)) window.set_close();
        if (window.is_key_down(Window::KeyCode::KEY_R)) rvpt.reload_shaders();
        if (window.is_key_down(Window::KeyCode::KEY_V)) rvpt.toggle_debug();
        if (window.is_key_up(Window::KeyCode::KEY_ENTER))
        {
            window.set_mouse_window_lock(!window.is_mouse_locked_to_window());
        }

        update_camera(window, rvpt);
        ImGui::NewFrame();
        rvpt.update_imgui();
        rvpt.update();
        rvpt.draw();
    }
    rvpt.shutdown();

    return 0;
}
