enum eVertexArrayObject {
    VAOScene,
    VAOCount
};

enum eBufferObject {
    VBOScene,
    BOCount
};

enum eProgram {
    SceneProgram,
    ProgramCount
};

enum eTexture {
    NoTexture,
    TextureCount
};

#include <common.cpp>

GLchar windowTitle[] = "Borsi Barnabas CWZ079 SZG2";

GLint dragged = -1; // -1 = nincs huzas, n = huzott pont indexe
GLfloat pointRadius = 0.1f; // kontrollpont hitbox merete
GLfloat pointSizePx = 9.0f; // pixel atmero
GLfloat lineWidth = 2.0f; // vonalak vastagsaga

vector<vec2> controlPoints;
vector<vec2> curveVertices;

vec3 curveColor(0.95f, 0.80f, 0.15f); // gorbe szine
vec3 pointColor(0.95f, 0.20f, 0.25f); // kontrollpont szine
vec3 polygonColor(0.20f, 0.80f, 0.95f); // vonal szine
vec4 backgroundColor(0.08f, 0.08f, 0.08f, 1.0f); // hatter szine

GLuint colorLocation = 0;
GLfloat aspectRatio = 1.0f;

// -------------------------------------------

vec2 screenToWorld(double xPos, double yPos) {
    dvec2 mousePosition;
    mousePosition.x = xPos * 2.0 / (GLdouble)windowWidth - 1.0;
    mousePosition.y = ((GLdouble)windowHeight - yPos) * 2.0 / (GLdouble)windowHeight - 1.0;

    if (windowWidth < windowHeight) mousePosition.y /= aspectRatio;
    else mousePosition.x *= aspectRatio;

    return vec2(mousePosition);
}

GLint getActivePoint(const vector<vec2>& points, GLfloat sensitivity, vec2 mousePosition) {
    GLfloat sensitivitySquare = sensitivity * sensitivity;
    for (GLint i = 0; i < (GLint)points.size(); i++)
        if (dot(points[i] - mousePosition, points[i] - mousePosition) < sensitivitySquare)
            return i;
    return -1;
}

vec2 deCasteljau(const vector<vec2>& points, float t) {
    if (points.empty()) return vec2(0.0f);

    vector<vec2> tmp = points;
    int n = (int)tmp.size();

    for (int level = 1; level < n; ++level) {
        for (int i = 0; i < n - level; ++i) {
            tmp[i] = (1.0f - t) * tmp[i] + t * tmp[i + 1];
        }
    }

    return tmp[0];
}

void rebuildCurve() {
    curveVertices.clear();

    if (controlPoints.size() == 1) {
        curveVertices.push_back(controlPoints[0]);
        return;
    }

    if (controlPoints.size() < 2) return;

    // Minel tobb pont van, annal reszletesebb torott vonallal kozelitjuk a gorbet
    int segments = std::max(128, (int)controlPoints.size() * 64);
    curveVertices.reserve(segments + 1);

    for (int i = 0; i <= segments; ++i) {
        float t = (float)i / (float)segments;
        curveVertices.push_back(deCasteljau(controlPoints, t));
    }
}

void uploadVertices(const vector<vec2>& vertices) {
    glBindVertexArray(VAO[VAOScene]);
    glBindBuffer(GL_ARRAY_BUFFER, BO[VBOScene]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec2), vertices.empty() ? nullptr : vertices.data(), GL_DYNAMIC_DRAW);
}

// -------------------------------------------

void initShaderProgram() {
    ShaderInfo shader_info[ProgramCount][3] = { {
        { GL_VERTEX_SHADER,   "./vertexShader.glsl" },
        { GL_FRAGMENT_SHADER, "./fragmentShader.glsl" },
        { GL_NONE, nullptr }
    } };

    for (int programItem = 0; programItem < ProgramCount; programItem++) {
        program[programItem] = LoadShaders(shader_info[programItem]);
        locationMatModel = glGetUniformLocation(program[programItem], "matModel");
        locationMatView = glGetUniformLocation(program[programItem], "matView");
        locationMatProjection = glGetUniformLocation(program[programItem], "matProjection");
    }

    glBindVertexArray(VAO[VAOScene]);
    glBindBuffer(GL_ARRAY_BUFFER, BO[VBOScene]);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), 0);

    glUseProgram(program[SceneProgram]);

    colorLocation = glGetUniformLocation(program[SceneProgram], "baseColor");

    matModel = mat4(1.0f);
    matView = lookAt(vec3(0.0f, 0.0f, 9.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

    glUniformMatrix4fv(locationMatModel, 1, GL_FALSE, value_ptr(matModel));
    glUniformMatrix4fv(locationMatView, 1, GL_FALSE, value_ptr(matView));
    glUniformMatrix4fv(locationMatProjection, 1, GL_FALSE, value_ptr(matProjection));

    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);

    glEnable(GL_POINT_SMOOTH);
    glPointSize(pointSizePx);
    glLineWidth(lineWidth);
}

// -------------------------------------------

void display(GLFWwindow* window, double currentTime) {
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program[SceneProgram]);

    // Kontrollpoligon
    if (controlPoints.size() >= 2) {
        glUniform3fv(colorLocation, 1, value_ptr(polygonColor));
        uploadVertices(controlPoints);
        glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)controlPoints.size());
    }

    // Gorbe
    if (curveVertices.size() >= 2) {
        glUniform3fv(colorLocation, 1, value_ptr(curveColor));
        uploadVertices(curveVertices);
        glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)curveVertices.size());
    }

    // Kontrollpontok
    if (!controlPoints.empty()) {
        glUniform3fv(colorLocation, 1, value_ptr(pointColor));
        uploadVertices(controlPoints);
        glDrawArrays(GL_POINTS, 0, (GLsizei)controlPoints.size());
    }
}

// -------------------------------------------

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    windowWidth = glm::max(width, 1);
    windowHeight = glm::max(height, 1);
    aspectRatio = (float)windowWidth / (float)windowHeight;
    glViewport(0, 0, windowWidth, windowHeight);

    if (windowWidth < windowHeight)
        matProjection = ortho(-worldSize, worldSize, -worldSize / aspectRatio, worldSize / aspectRatio, -100.0, 100.0);
    else
        matProjection = ortho(-worldSize * aspectRatio, worldSize * aspectRatio, -worldSize, worldSize, -100.0, 100.0);

    glUseProgram(program[SceneProgram]);
    glUniformMatrix4fv(locationMatProjection, 1, GL_FALSE, value_ptr(matProjection));
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if ((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (action == GLFW_PRESS)
        keyboard[key] = GL_TRUE;
    else if (action == GLFW_RELEASE)
        keyboard[key] = GL_FALSE;

    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        controlPoints.clear();
        curveVertices.clear();
        dragged = -1;
    }
}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
    if (dragged >= 0 && dragged < (GLint)controlPoints.size()) {
        controlPoints[dragged] = screenToWorld(xPos, yPos);
        rebuildCurve();
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    vec2 mousePosition = screenToWorld(xPos, yPos);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        GLint hit = getActivePoint(controlPoints, pointRadius, mousePosition);

        if (hit >= 0) {
            dragged = hit;
        }
        else {
            controlPoints.push_back(mousePosition);
            dragged = (GLint)controlPoints.size() - 1;
            rebuildCurve();
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        dragged = -1;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        GLint hit = getActivePoint(controlPoints, pointRadius, mousePosition);
        if (hit >= 0) {
            controlPoints.erase(controlPoints.begin() + hit);
            if (dragged == hit) dragged = -1;
            else if (dragged > hit) dragged--;
            rebuildCurve();
        }
    }
}

// -------------------------------------------

int main(void) {
    init(3, 3, GLFW_OPENGL_COMPAT_PROFILE);
    initShaderProgram();
    framebufferSizeCallback(window, windowWidth, windowHeight);
    setlocale(LC_ALL, "");

    cout << "Bezier Curve Editor" << endl << endl;
    cout << "Vezerles:" << endl;
    cout << "  Bal kattintas ures helyre : uj kontrollpont letrehozasa" << endl;
    cout << "  Bal gomb huzasa ponton    : kontrollpont mozgatasa" << endl;
    cout << "  Jobb kattintas ponton     : kontrollpont torlese" << endl;
    cout << "  C                         : osszes pont torlese" << endl;
    cout << "  ESC                       : kilepes" << endl << endl;

    while (!glfwWindowShouldClose(window)) {
        display(window, glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanUpScene(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}
