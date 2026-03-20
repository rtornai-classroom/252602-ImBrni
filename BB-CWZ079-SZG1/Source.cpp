enum eVertexArrayObject {
    VAOCirclePoint,
    VAOSegmentPoint,
    VAOCount
};

enum eBufferObject {
    VBOCirclePoint,
    VBOSegmentPoint,
    BOCount
};

enum eProgram {
    CircleProgram,
    ProgramCount
};

enum eTexture {
    NoTexture,
    TextureCount
};

#include <common.cpp>

GLchar windowTitle[] = "Borsi Barnabas CWZ079 SZG1";

GLuint objectType; // 0 = segment, 1 = circle

GLuint circleCenter;
GLuint circleRadius;
GLuint circleCenterColor;
GLuint circleEdgeColor;

GLuint segmentColor;
GLuint segmentLength;
GLuint segmentHeight;

constexpr float WINDOW_SIZE = 600.0f;
constexpr float CIRCLE_RADIUS = 50.0f;
constexpr float SEGMENT_LENGTH = WINDOW_SIZE / 3.0f;
constexpr float SEGMENT_THICKNESS = 3.0f;
constexpr float SEGMENT_MOVE_STEP = 10.0f;

vec2 circlePoint(WINDOW_SIZE * 0.5f, WINDOW_SIZE * 0.5f);
vec2 segmentPoint(WINDOW_SIZE * 0.5f, WINDOW_SIZE * 0.5f);
vec2 circleVelocity(180.0f, 0.0f);

vec3 redColor(0.95f, 0.10f, 0.10f);
vec3 greenColor(0.10f, 0.75f, 0.10f);
vec3 blueColor(0.10f, 0.20f, 0.95f);
vec3 yellowColor(1.0f, 0.92f, 0.20f);

double previousTime = 0.0;

inline void updateCirclePointBuffer() {
    glBindBuffer(GL_ARRAY_BUFFER, BO[VBOCirclePoint]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2), value_ptr(circlePoint));
}

inline void updateSegmentPointBuffer() {
    glBindBuffer(GL_ARRAY_BUFFER, BO[VBOSegmentPoint]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2), value_ptr(segmentPoint));
}

inline void moveSegment(float dist) {
    float minCenterY = SEGMENT_THICKNESS * 0.5f;
    float maxCenterY = WINDOW_SIZE - SEGMENT_THICKNESS * 0.5f;
    segmentPoint.y = clamp(segmentPoint.y + dist, minCenterY, maxCenterY);
    updateSegmentPointBuffer();
}

inline bool circleIntersectsSegment() {
    float cx = clamp(circlePoint.x, segmentPoint.x - SEGMENT_LENGTH * 0.5f, segmentPoint.x + SEGMENT_LENGTH * 0.5f);
    float cy = clamp(circlePoint.y, segmentPoint.y - SEGMENT_THICKNESS * 0.5f, segmentPoint.y + SEGMENT_THICKNESS * 0.5f);

    float dx = circlePoint.x - cx;
    float dy = circlePoint.y - cy;

    return ( dx * dx + dy * dy ) <= CIRCLE_RADIUS * CIRCLE_RADIUS;
}

void initShaderProgram() {
    ShaderInfo shaderInfo[] = {
        { GL_FRAGMENT_SHADER, "./fragmentShader.glsl" },
        { GL_GEOMETRY_SHADER, "./geometryShader.glsl" },
        { GL_VERTEX_SHADER, "./vertexShader.glsl" },
        { GL_NONE, nullptr }
    };

    program[CircleProgram] = LoadShaders(shaderInfo);
    glUseProgram(program[CircleProgram]);

    glBindVertexArray(VAO[VAOCirclePoint]);
    glBindBuffer(GL_ARRAY_BUFFER, BO[VBOCirclePoint]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(VAO[VAOSegmentPoint]);
    glBindBuffer(GL_ARRAY_BUFFER, BO[VBOSegmentPoint]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    locationMatProjection = glGetUniformLocation(program[CircleProgram], "matProjection");
    locationMatModelView = glGetUniformLocation(program[CircleProgram], "matModelView");

    objectType = glGetUniformLocation(program[CircleProgram], "objectType");
    
    segmentColor = glGetUniformLocation(program[CircleProgram], "segmentColor");
    segmentLength = glGetUniformLocation(program[CircleProgram], "segmentLength");
    segmentHeight = glGetUniformLocation(program[CircleProgram], "segmentHeight");
    updateSegmentPointBuffer();

    circleCenter = glGetUniformLocation(program[CircleProgram], "circleCenter");
    circleRadius = glGetUniformLocation(program[CircleProgram], "circleRadius");
    circleCenterColor = glGetUniformLocation(program[CircleProgram], "centerColor");
    circleEdgeColor = glGetUniformLocation(program[CircleProgram], "edgeColor");
    updateCirclePointBuffer();
}

void updateAnimation(double deltaTime)
{
    circlePoint.x += circleVelocity.x * static_cast<float>(deltaTime);

    if (circlePoint.x > WINDOW_SIZE - CIRCLE_RADIUS) {
        circlePoint.x = WINDOW_SIZE - CIRCLE_RADIUS;
        circleVelocity.x = -circleVelocity.x;
    }

    if (circlePoint.x < CIRCLE_RADIUS) {
        circlePoint.x = CIRCLE_RADIUS;
        circleVelocity.x = -circleVelocity.x;
    }

    updateCirclePointBuffer();
}


void display(GLFWwindow* window, double currentTime) {
    glClear(GL_COLOR_BUFFER_BIT);

    double deltaTime = currentTime - previousTime;
    previousTime = currentTime;
    updateAnimation(deltaTime);

    bool intersects = circleIntersectsSegment();
    vec3 currentCenterColor = intersects ? redColor : greenColor;
    vec3 currentEdgeColor   = intersects ? greenColor : redColor;

    glUseProgram(program[CircleProgram]);
    matModel = mat4(1.0f);
    matView = mat4(1.0f);
    matModelView = matView * matModel;

    glUniformMatrix4fv(locationMatModelView, 1, GL_FALSE, value_ptr(matModelView));
    glUniformMatrix4fv(locationMatProjection, 1, GL_FALSE, value_ptr(matProjection));
    
    glBindVertexArray(VAO[VAOSegmentPoint]);
    glUniform1i(objectType, 0);
    glUniform1f(segmentLength, SEGMENT_LENGTH);
    glUniform1f(segmentHeight, SEGMENT_THICKNESS);
    glUniform3fv(segmentColor, 1, value_ptr(blueColor));
    glDrawArrays(GL_POINTS, 0, 1);

    glBindVertexArray(VAO[VAOCirclePoint]);
    glUniform1i(objectType, 1);
    glUniform2fv(circleCenter, 1, value_ptr(circlePoint));
    glUniform1f(circleRadius, CIRCLE_RADIUS);
    glUniform3fv(circleCenterColor, 1, value_ptr(currentCenterColor));
    glUniform3fv(circleEdgeColor, 1, value_ptr(currentEdgeColor));
    glDrawArrays(GL_POINTS, 0, 1);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    windowWidth = glm::max(width, 1);
    windowHeight = glm::max(height, 1);
    glViewport(0, 0, windowWidth, windowHeight);

    matProjection = ortho(0.0f, WINDOW_SIZE, 0.0f, WINDOW_SIZE, -1.0f, 1.0f);
    matModel = mat4(1.0f);
    matView = mat4(1.0f);
    matModelView = matView * matModel;

    glUseProgram(program[CircleProgram]);
    glUniformMatrix4fv(locationMatModelView, 1, GL_FALSE, value_ptr(matModelView));
    glUniformMatrix4fv(locationMatProjection, 1, GL_FALSE, value_ptr(matProjection));
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_UP) {
            moveSegment(SEGMENT_MOVE_STEP);
        }
        else if (key == GLFW_KEY_DOWN) {
            moveSegment(-SEGMENT_MOVE_STEP);
        }
    }
}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {}
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {}

int main(void) {
    init(3, 3, GLFW_OPENGL_COMPAT_PROFILE);

    glfwSetWindowAspectRatio(window, 1, 1);
    glfwSetWindowSize(window, (int)WINDOW_SIZE, (int)WINDOW_SIZE);
    glfwSetWindowSizeLimits(window, (int)WINDOW_SIZE, (int)WINDOW_SIZE, (int)WINDOW_SIZE, (int)WINDOW_SIZE);

    initShaderProgram();

    glClearColor(yellowColor.r, yellowColor.g, yellowColor.b, 1.0f);

    framebufferSizeCallback(window, (int)WINDOW_SIZE, (int)WINDOW_SIZE);
    previousTime = glfwGetTime();

    setlocale(LC_ALL, "");
    cout << "ESC\tkilepes" << endl;
    cout << "FEL\tszakasz mozgatasa felfele" << endl;
    cout << "LE\tszakasz mozgatasa lefele" << endl;

    while (!glfwWindowShouldClose(window)) {
        display(window, glfwGetTime());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanUpScene(0);
    return 0;
}
