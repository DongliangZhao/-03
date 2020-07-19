

#include "GLTools.h"
#include "GLMatrixStack.h"
#include "GLFrame.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLGeometryTransform.h"

#include <math.h>
#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif

// 各种需要的类
GLShaderManager         shaderManager;
GLMatrixStack           modelViewMatrix;    // 模型视图矩阵
GLMatrixStack           projectionMatrix;   // 投影矩阵
GLFrame                 objectFrame;
GLFrame                 cameraFrame;


// 投影矩阵
GLFrustum               viewFrustum;


// 容器类
GLBatch                 pointBatch;
GLBatch                 lineBatch;
GLBatch                 lineStripBatch;
GLBatch                 lineLoopBatch;
GLBatch                 triangleBatch;
GLBatch                 triangleStripBatch;
GLBatch                 triangleFanBatch;

// 几何变换管道
GLGeometryTransform     transformPipeline;

GLfloat vGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
GLfloat vBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };

// 定义跟踪效果步骤
int nStep = 0;


//为程序作一次性的设置
void SetupRC()
{
    
    //设置背影颜色
    glClearColor(0.7f, 0.7f, 0.8f, 1.0);
    
    // 初始化着色器
    shaderManager.InitializeStockShaders();
    
    // 设置变换管道以使用两个矩阵堆栈
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
    // 修改观察者位置
    cameraFrame.MoveForward(-15.0f);
    
    //设置顶点，三角形形状
    GLfloat vVerts[] = {
        4,4,0,
        0,4,0,
        4,0,0
    };
    // 提交批次类
    // 用点的形式
    pointBatch.Begin(GL_POINTS, 3);
    pointBatch.CopyVertexData3f(vVerts);
    pointBatch.End();
    
    // 线
    lineBatch.Begin(GL_LINES, 3);
    lineBatch.CopyVertexData3f(vVerts);
    lineBatch.End();
    
    // 线段的形式
    lineStripBatch.Begin(GL_LINE_STRIP, 3);
    lineStripBatch.CopyVertexData3f(vVerts);
    lineStripBatch.End();
    
    // 线环
    lineLoopBatch.Begin(GL_LINE_LOOP, 3);
    lineLoopBatch.CopyVertexData3f(vVerts);
    lineLoopBatch.End();
    
    // 金字塔
    GLfloat vPyramid[12][3] = {
        -2.0f, 0.0f, -2.0f,
        2.0f, 0.0f, -2.0f,
        0.0f, 4.0f, 0.0f,
        
        2.0f, 0.0f, -2.0f,
        2.0f, 0.0f, 2.0f,
        0.0f, 4.0f, 0.0f,
        
        2.0f, 0.0f, 2.0f,
        -2.0f, 0.0f, 2.0f,
        0.0f, 4.0f, 0.0f,
        
        -2.0f, 0.0f, 2.0f,
        -2.0f, 0.0f, -2.0f,
        0.0f, 4.0f, 0.0f
        
    };
    
    triangleBatch.Begin(GL_TRIANGLES, 12);
    triangleBatch.CopyVertexData3f(vPyramid);
    triangleBatch.End();
    
    // 六边形/
    GLfloat vPoints[100][3];
    int nVerts = 0;
    // 半径
    GLfloat r = 4.0f;
    //原点(x,y,z) = (0,0,0);
    vPoints[nVerts][0] = 0.0f;
    vPoints[nVerts][1] = 0.0f;
    vPoints[nVerts][2] = 0.0f;
    
    // 修改24，这里六边形应该为6， triangleFanBatch.Begin(GL_TRIANGLE_FAN, 26);26 应为8，修改对应的值，可变成圆
    for (GLfloat angle = 0; angle < M3D_2PI; angle += M3D_2PI/24) {
        // 数组下标自增，每自增1次表示一个顶点
        nVerts++;
        
        // 弧长 = 半径 * 角度（弧度制），知道cos，角度=arccos,
        // x点坐标 cos(angle) * 半径
        vPoints[nVerts][0] = float(cos(angle)) * r;
        // y点坐标 cos(angle) * 半径
        vPoints[nVerts][1] = float(sin(angle)) * r;
        // z点坐标
        vPoints[nVerts][2] = - 2.0f;
    }
    
    // 结束扇形
    nVerts++;
    vPoints[nVerts][0] = r;
    vPoints[nVerts][1] = 0;
    vPoints[nVerts][2] = 0.0f;
    
    // 加载
    triangleFanBatch.Begin(GL_TRIANGLE_FAN, 26);
    triangleFanBatch.CopyVertexData3f(vPoints);
    triangleFanBatch.End();
    
    // 三角形环
    //三角形条带，一个小环或圆柱段
    //顶点下标
    int iCounter = 0;
    //半径
    GLfloat radius = 3.0f;
    //从0度~360度，以0.3弧度为步长
    for(GLfloat angle = 0.0f; angle <= (2.0f*M3D_PI); angle += 0.3f)
    {
        //或许圆形的顶点的X,Y
        GLfloat x = radius * sin(angle);
        GLfloat y = radius * cos(angle);
        
        //绘制2个三角形（他们的x,y顶点一样，只是z点不一样）
        vPoints[iCounter][0] = x;
        vPoints[iCounter][1] = y;
        vPoints[iCounter][2] = -0.5;
        iCounter++;
        
        vPoints[iCounter][0] = x;
        vPoints[iCounter][1] = y;
        vPoints[iCounter][2] = 0.5;
        iCounter++;
    }
    
    // 关闭循环
    printf("三角形带的顶点数：%d\n",iCounter);
    //结束循环，在循环位置生成2个三角形
    vPoints[iCounter][0] = vPoints[0][0];
    vPoints[iCounter][1] = vPoints[0][1];
    vPoints[iCounter][2] = -0.5;
    iCounter++;
    
    vPoints[iCounter][0] = vPoints[1][0];
    vPoints[iCounter][1] = vPoints[1][1];
    vPoints[iCounter][2] = 0.5;
    iCounter++;
    
    // GL_TRIANGLE_STRIP 共用一个条带（strip）上的顶点的一组三角形
    triangleStripBatch.Begin(GL_TRIANGLE_STRIP, iCounter);
    triangleStripBatch.CopyVertexData3f(vPoints);
    triangleStripBatch.End();
    
    
}
void DrawWireFramedBatch(GLBatch* pBatch){
    
    /*------------画绿色部分----------------*/
    /* GLShaderManager 中的Uniform 值——平面着色器
     参数1：平面着色器
     参数2：运行为几何图形变换指定一个 4 * 4变换矩阵
     --transformPipeline 变换管线（指定了2个矩阵堆栈）
     参数3：颜色值
     */
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vGreen);
    pBatch->Draw();
    
    /*-----------边框部分-------------------*/
    /*
     glEnable(GLenum mode); 用于启用各种功能。功能由参数决定
     参数列表：http://blog.csdn.net/augusdi/article/details/23747081
     注意：glEnable() 不能写在glBegin() 和 glEnd()中间
     GL_POLYGON_OFFSET_LINE  根据函数glPolygonOffset的设置，启用线的深度偏移
     GL_LINE_SMOOTH          执行后，过虑线点的锯齿
     GL_BLEND                启用颜色混合。例如实现半透明效果
     GL_DEPTH_TEST           启用深度测试 根据坐标的远近自动隐藏被遮住的图形（材料
     
     
     glDisable(GLenum mode); 用于关闭指定的功能 功能由参数决定
     
     */
    
    //画黑色边框
    glPolygonOffset(-1.0f, -1.0f);// 偏移深度，在同一位置要绘制填充和边线，会产生z冲突，所以要偏移
    glEnable(GL_POLYGON_OFFSET_LINE);
    
    // 画反锯齿，让黑边好看些
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //绘制线框几何黑色版 三种模式，实心，边框，点，可以作用在正面，背面，或者两面
    //通过调用glPolygonMode将多边形正面或者背面设为线框模式，实现线框渲染
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    //设置线条宽度
    glLineWidth(2.5f);
    
    /* GLShaderManager 中的Uniform 值——平面着色器
     参数1：平面着色器
     参数2：运行为几何图形变换指定一个 4 * 4变换矩阵
     --transformPipeline.GetModelViewProjectionMatrix() 获取的
     GetMatrix函数就可以获得矩阵堆栈顶部的值
     参数3：颜色值（黑色）
     */
    
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(), vBlack);
    pBatch->Draw();
    
    // 复原原本的设置
    //通过调用glPolygonMode将多边形正面或者背面设为全部填充模式
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glLineWidth(1.0f);
    glDisable(GL_BLEND);
    glDisable(GL_LINE_SMOOTH);
    
    
}

//开始渲染
void RenderScene(void) {
    
    // 清除一个或者多个特定的缓存区
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    // 压栈
    modelViewMatrix.PushMatrix();
    
    // 构建观察者矩阵
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    
    // 矩阵相乘，结果存到矩阵堆栈的顶部
    // 栈顶单元矩阵 *  mCameram  = newCamera
    modelViewMatrix.MultMatrix(mCamera);
    
    // 物体矩阵
    M3DMatrix44f mObjectFrame;
    objectFrame.GetMatrix(mObjectFrame);
    
    // 矩阵相乘，放到模型视图矩阵，结果存到矩阵堆栈的顶部
    // newCamera * mObjectFrame
    modelViewMatrix.MultMatrix(mObjectFrame);
    
    // 模型视图矩阵 投影矩阵 mvp
    /* GLShaderManager 中的Uniform 值——平面着色器
     参数1：平面着色器
     参数2：运行为几何图形变换指定一个 4 * 4变换矩阵
     --transformPipeline.GetModelViewProjectionMatrix() 获取的
     GetMatrix函数就可以获得矩阵堆栈顶部的值
     参数3：颜色值（黑色）
     */
    shaderManager.UseStockShader(GLT_SHADER_FLAT, transformPipeline.GetModelViewProjectionMatrix(),vBlack);
    
    
    switch (nStep) {
        case 0:
            // 设置点的大小
            glPointSize(4.0f);
            pointBatch.Draw();
            glPointSize(1.0f);
            break;
        case 1:
            // 设置线宽
            glLineWidth(4.0f);
            lineBatch.Draw();
            glLineWidth(1.0f);
            break;
        case 2:
            
            glLineWidth(4.0f);
            lineStripBatch.Draw();
            glLineWidth(1.0f);
            break;
        case 3:
            glLineWidth(4.0f);
            lineLoopBatch.Draw();
            glLineWidth(1.0f);
            
            break;
        case 4:
            DrawWireFramedBatch(&triangleBatch);
            break;
        case 5:
            DrawWireFramedBatch(&triangleFanBatch);
            break;
        case 6:
            DrawWireFramedBatch(&triangleStripBatch);
            break;
        default:
            break;
    }
    // 还原到以前的模型视图矩阵
    modelViewMatrix.PopMatrix();
    // 交换缓冲区
    glutSwapBuffers();
    
}



//窗口大小改变时接受新的宽度和高度，其中0,0代表窗口中视口的左下角坐标，w，h代表像素
void ChangeSize(int w,int h) {
    // 设置视口
    glViewport(0,0, w, h);
    // 创建投影矩阵，并将其载入投影矩阵堆栈中
    /*
    参数1：垂直方向上的视场角度
    参数2：视口纵横比 = w/h
    参数3：近裁剪面距离
    参数4：远裁剪面距离
    */
    viewFrustum.SetPerspective(35.0f, floor(w)/floor(h), 1.0f, 500.0f);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    
    // 加载单元矩阵
    modelViewMatrix.LoadIdentity();
}
void KeyPressFunc(unsigned char key, int x, int y){
    
    if(key == 32)
    {
        nStep++;
        
        if(nStep > 6)
            nStep = 0;
    }
    
    switch(nStep)
    {
        case 0:
            glutSetWindowTitle("GL_POINTS");
            break;
        case 1:
            glutSetWindowTitle("GL_LINES");
            break;
        case 2:
            glutSetWindowTitle("GL_LINE_STRIP");
            break;
        case 3:
            glutSetWindowTitle("GL_LINE_LOOP");
            break;
        case 4:
            glutSetWindowTitle("GL_TRIANGLES");
            break;
        case 5:
            glutSetWindowTitle("GL_TRIANGLE_STRIP");
            break;
        case 6:
            glutSetWindowTitle("GL_TRIANGLE_FAN");
            break;
    }
    
    // 手动触发渲染
    glutPostRedisplay();
}
void SpecialKeys(int key, int x, int y){
    
    if(key == GLUT_KEY_UP)
        //围绕一个指定的X,Y,Z轴旋转。
        objectFrame.RotateWorld(m3dDegToRad(-5.0f), 1.0f, 0.0f, 0.0f);
    
    if(key == GLUT_KEY_DOWN)
        objectFrame.RotateWorld(m3dDegToRad(5.0f), 1.0f, 0.0f, 0.0f);
    
    if(key == GLUT_KEY_LEFT)
        objectFrame.RotateWorld(m3dDegToRad(-5.0f), 0.0f, 1.0f, 0.0f);
    
    if(key == GLUT_KEY_RIGHT)
        objectFrame.RotateWorld(m3dDegToRad(5.0f), 0.0f, 1.0f, 0.0f);
    
    // 手动触发渲染
    glutPostRedisplay();
}


int main(int argc,char* argv[])
{
    // 设置当前工作目录，针对Mac OSX
    gltSetWorkingDirectory(argv[0]);
    // 初始化GLUT库
    glutInit(&argc, argv);
    // 初始化双缓存窗口 双缓冲窗口，RGBA颜色模式，深度测试，模板缓冲区
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_STENCIL);
    
    
    // 设置窗口大小，窗口标题
    glutInitWindowSize(800, 600);
    glutCreateWindow("图元连接案例");
    
    //注册回调函数（改变尺寸）
    glutReshapeFunc(ChangeSize);
    //点击空格时，调用的函数
    glutKeyboardFunc(KeyPressFunc);
    //特殊键位函数（上下左右）
    glutSpecialFunc(SpecialKeys);
    //显示函数
    glutDisplayFunc(RenderScene);
    
    // 驱动程序的初始化中没有出现任何问题
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr,"glew error:%s\n",glewGetErrorString(err));
        
        return 1;
    }
    
    // 调用SetupRC绘制
    SetupRC();
    // runloop运行循环
    glutMainLoop();
    return 0;
    
}

