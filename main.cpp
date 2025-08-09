#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "fillpoly.h"

#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> 

#include <iostream>

static void glfw_error_callback(int error, const char* description){
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

constexpr auto vertex_size = 8;

std::vector<in_point> get_absolute_points(std::vector<in_point> &points, const ImVec2 &canvas_p0){
    std::vector<in_point> absolute_points;

    for(auto p: points){
        auto np = p;

        np.x += canvas_p0.x;
        np.y += canvas_p0.y;

        absolute_points.push_back(np);
    }

    return absolute_points;
}

void draw_lines(std::vector<in_point> &points){
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    for(size_t i = 0; i + 1 < points.size(); ++i){
        auto p1 = ImVec2(points[i].x, points[i].y);
        auto p2 = ImVec2(points[i+1].x, points[i+1].y);

        draw_list->AddLine(p1, p2, IM_COL32(0, 0, 255, 255));
    }
}

void draw_vertexes(std::vector<in_point> &points){
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    for(auto &cur_p: points){
        ImVec2 min_base = {(float)cur_p.x - vertex_size, (float)cur_p.y - vertex_size};

        ImVec2 max_base = {(float)cur_p.x + vertex_size, (float)cur_p.y + vertex_size};

        auto color = ImGui::ColorConvertFloat4ToU32(
            {cur_p.col.r, cur_p.col.g, cur_p.col.b, cur_p.col.a}
        );

        draw_list->AddRectFilled(min_base, max_base, color);
    }
}

void draw_fillpoly(const std::vector<in_point> &points){
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    auto points_to_draw = fillpoly(points);

    for(auto &point: points_to_draw){
        auto color = ImGui::ColorConvertFloat4ToU32(
            {point.col.r, point.col.g, point.col.b, point.col.a}
        );

        draw_list->AddRectFilled(ImVec2(point.x, point.y), ImVec2(point.x+1, point.y+1), color);
    }
}

void draw(std::vector<in_point> &points, const ImVec2 &canvas_p0, const ImVec2 &canvas_total, bool finished){
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    draw_list->AddRectFilled(canvas_p0, 
        ImVec2(canvas_p0.x + canvas_total.x, canvas_p0.y + canvas_total.y), 
        IM_COL32(255, 255, 255, 255)
    );

    auto abs_points = get_absolute_points(points, canvas_p0);
    
    draw_vertexes(abs_points);
    
    draw_lines(abs_points);

    if(finished && abs_points.size() >= 3){
        auto p1 = ImVec2(abs_points[0].x, abs_points[0].y);
        auto p2 = ImVec2(abs_points.back().x, abs_points.back().y);
        
        draw_list->AddLine(p1, p2, IM_COL32(0, 0, 255, 255));

        draw_fillpoly(abs_points); 
    }
}

GLFWwindow *create_window(){
    glfwSetErrorCallback(glfw_error_callback);
    
    if (!glfwInit()){
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "glfw window", nullptr, nullptr);

    if (window == nullptr){
        return window;
    }

    return window;
}

void destroy_window(GLFWwindow *window){
    glfwDestroyWindow(window);
    glfwTerminate();
}

ImGuiIO &imgui_configure(){
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor())*2;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    io.ConfigViewportsNoAutoMerge = true;
    io.ConfigViewportsNoTaskBarIcon = true;

    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;
    io.ConfigDpiScaleFonts = true;
    io.ConfigDpiScaleViewports = true;

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable){
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    return io;
}

void imgui_opengl_init(GLFWwindow *window){
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();
}

void imgui_opengl_uninit(){
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

float color[4] = {1,1,1,1};

void render(bool &main_window, bool &color_window, bool &add_enabled, ImVec2 &cur_point, std::vector<in_point> &vertexes){
    ImGui::Begin("Fillpoly", &main_window);

    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();
    ImVec2 canvas_total = ImVec2(ImGui::GetWindowWidth() - 35, 500);

    auto set_point = [&]() {
        color_window = true;

        cur_point = ImGui::GetMousePos();
        cur_point.x -= canvas_p0.x;
        cur_point.y -= canvas_p0.y;
    };

    if(color_window){
        ImGui::Begin("Color", &color_window);

        ImGui::ColorPicker4("CorPicker", color);

        if(!color_window){
            in_point out{(int)cur_point.x, (int)cur_point.y, color};

            vertexes.push_back(out);
        }

        ImGui::End();
    }

    bool finished = !color_window && !add_enabled;

    ImGui::BeginDisabled(finished);

    if(ImGui::InvisibleButton("my_canvas", canvas_total)){
        set_point();
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)){
        set_point();

        if(vertexes.size() + 1 >= 3){
            add_enabled = false;
        }
    }

    ImGui::EndDisabled();

    draw(vertexes, canvas_p0, canvas_total, finished);

    if(ImGui::Button("Clear")){
        vertexes.clear();
        add_enabled = true;
    }

    ImGui::End();
}

int main(int, char**){ 
    std::vector<in_point> vertexes;

    auto window = create_window();

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    auto io = imgui_configure();
    imgui_opengl_init(window);

    bool main_window = true;
    bool color_picker_open = false;
    bool add_enabled = true;

    ImVec2 cur_point;

    while (!glfwWindowShouldClose(window)){
        glfwPollEvents();

        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0){
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        if(!main_window){
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        render(main_window, color_picker_open, add_enabled, cur_point, vertexes);

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable){
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);
    }

    imgui_opengl_uninit();
    destroy_window(window);

    return 0;
}

