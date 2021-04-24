//********************
// Author:  yh 
// Time:    2021/4/24.
// 
//********************
//

/*1  画直线 line 

void line(InputOutputArray img, Point pt1, Point pt2, const Scalar& color,int thickness = 1, int lineType = LINE_8, int shift = 0);
img:输入图像，直线画在该图像上
        pt1:直线的起点
        pt2:直线的终点
        color:直线的颜色
        thickness:直线的大小
        lineType：直线类型
        shift:直线的偏移量

２ 画带箭头的直线 arrowedLine

void arrowedLine(InputOutputArray img, Point pt1, Point pt2, const Scalar& color, int thickness=1, int line_type=8, int shift=0, double tipLength=0.1);

img:输入图像，直线画在该图像上
        pt1:直线的起点
        pt2:直线的终点
        color:直线的颜色
        thickness:直线的大小
        lineType：直线类型
        shift:直线的偏移量
        tipLength:箭头占线段的比例

３ 画矩形

void rectangle(CV_IN_OUT Mat& img, Rect rec, const Scalar& color, int thickness = 1, int lineType = LINE_8, int shift = 0);

img:输入图像，矩形画在该图像上
        rec:矩形
        color:矩形的颜色
        thickness:矩形边的大小
        lineType：矩形类型
        shift:直线的偏移量

４  画圆形

void circle(InputOutputArray img, Point center, int radius,　const Scalar& color, int thickness = 1,　int lineType = LINE_8, int shift = 0);
img:输入图像，圆形画在该图像上
        center:圆心
        color:圆形的颜色
        thickness:圆形边的大小
        lineType：圆形类型
        shift:圆形的偏移量

５  画椭圆

void ellipse(InputOutputArray img, Point center, Size axes, double angle, double startAngle, double endAngle,
             const Scalar& color, int thickness = 1, int lineType = LINE_8, int shift = 0);
img:输入图像，椭圆画在该图像上
        box:椭圆中心
        axes:椭圆的尺寸
        angle:椭圆的角度
        startAngle：画椭圆的开始角度
        endAngle:画椭圆的结束角度　
color:画椭的颜色
        thickness:画椭边的大小
        lineType：画椭类型
        shift:画椭的偏移量

６  填充多边形

void fillConvexPoly(Mat& img, const Point* pts, int npts, const Scalar& color, int lineType = LINE_8, int shift = 0);
img:输入图像，多边形填充在该图上
        pts:多边形的顶点集合
        npts:多边形的顶点个数
        color:填充多边形的颜色
        lineType：填充多边的画笔类型
        shift:多边形的偏移量

7  画轮廓点
void drawContours(InputOutputArray image, InputArrayOfArrays contours, int contourIdx, const Scalar& color, int thickness = 1, int lineType = LINE_8, InputArray hierarchy = noArray(), int maxLevel = INT_MAX, Point offset = Point() );
image:要绘制轮廓的图像
        contours:所有输入的轮廓，每个轮廓被保存成一个point向量
        contourIdx:指定要绘制轮廓的编号，如果是负数，则绘制所有的轮廓
        color:绘制轮廓所用的颜色
        thickness:绘制轮廓的线的粗细，如果是负数，则轮廓内部被填充
        lineType:绘制轮廓的线的连通性
        hierarchy:关于层级的可选参数，只有绘制部分轮廓时才会用到
        maxLeve:绘制轮廓的最高级别，这个参数只有hierarchy有效的时候才有效
        maxLevel=0，绘制与输入轮廓属于同一等级的所有轮廓即输入轮廓和与其相邻的轮廓
        maxLevel=1, 绘制与输入轮廓同一等级的所有轮廓与其子节点。
        maxLevel=2，绘制与输入轮廓同一等级的所有轮廓与其子节点以及子节点的子节点
        offset:轮廓点偏移量

8  显示文字
void putText(Mat& img, const string& text, Point org, int fontFace, double fontScale, Scalar color, int thickness=1, int lineType=8, bool bottomLeftOrigin=false);
img:输入图像，字显示在图上
        text:输出到图像的文字
        org:文字左下角位置。
fontFace:字体样式
        fontScale:字体大小
        color:字体颜色
        thickness:构成字体的线条宽度
        lineType:线条类型
*/

Mat srcImg(600, 800, CV_8UC3);
Mat matClone = srcImg.clone();
/*
line(srcImg, Point(100, 50), Point(600, 50), Scalar(0, 0, 255), 1, LINE_8, 0);

arrowedLine(srcImg, Point(100, 60), Point(600, 60), Scalar(0, 0, 255), 1, LINE_8, 0, 0.01);
*/

RNG g_rng(12345);
for (int  i = 0; i < 500; ++i) {

rectangle(srcImg, Rect(g_rng.uniform(0, 600), g_rng.uniform(0, 800), 100, 100), Scalar(g_rng.uniform(0, 255), g_rng.uniform(0, 255), g_rng.uniform(0, 255)));
}
/*
circle(srcImg, Point(300, 300), 100, Scalar(0, 0, 255));

ellipse(srcImg, Point(400, 400), Size(100, 60), -45, 0, 360, Scalar(0, 0, 255));
*/
/*
Point pts[5];
pts[0] = Point(500, 500);
pts[1] = Point(700, 500);
pts[2] = Point(750, 750);
pts[3] = Point(450, 750);
pts[4] = Point(500, 500);
fillConvexPoly(srcImg, pts, 5, Scalar(0, 255, 0), LINE_8, 0);
*/

/*
Mat img = imread(IMG_PATH_1);
vector<vector<Point> > vContours;
vector<Vec4i> hierarchy;
Mat matGray;
cvtColor(img, matGray, COLOR_BGR2GRAY);
Mat matBinary(img.rows, img.cols, CV_8UC1);
threshold(matGray, matBinary, 200, 255, THRESH_BINARY);
findContours(matBinary, vContours, hierarchy, RETR_LIST, CHAIN_APPROX_NONE, Point(0, 0));
drawContours(matClone, vContours, -1, Scalar(255, 0, 255));
 */


/*
//设置绘制文本的相关参数
std::string text = "opencv is open source";
int font_face = cv::FONT_HERSHEY_COMPLEX;
double font_scale = 2;
int thickness = 2;
int baseline;
//获取文本框的长宽
Size text_size = cv::getTextSize(text, font_face, font_scale, thickness, &baseline);

//将文本框居中绘制
Point origin;
origin.x = matClone.cols / 2 - text_size.width / 2;
origin.y = matClone.rows / 2 + text_size.height / 2;
putText(matClone, text, origin, font_face, font_scale, cv::Scalar(0, 255, 255), thickness, 8, 0);
*/


namedWindow(WINDOW_MAIN_NAME);
imshow(WINDOW_MAIN_NAME, srcImg);

//    namedWindow("Contours");
//    imshow("Contours", matClone);