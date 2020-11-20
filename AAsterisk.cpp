#include <stdio.h>
#include <vector>
#include <set>
#include <math.h>
#include <iostream>
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"


using namespace std;
using namespace cv;

Mat src;
int offset[][2] = {{-1, -1}, {0, -1}, {1, -1}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}};

enum heuristic
{
    FOUR_DIRECTION,
    EIGHT_DIRECTION,
    ANY_DIRECTION
};

struct pixels
{
    pixels();
    ~pixels();
    double getHeuristic(pixels* goal, int heuristicType);
    bool operator==(pixels* goal);
    pixels* parent;
    double f;
    double g;
    // double h;
    int value;
    int x;
    int y;
};

class AAsterisk
{
public:
    AAsterisk(Mat img, int heuristicType);
    ~AAsterisk();
    bool search();
    bool isValid(int x, int y);
    void setGoal(pixels* goal);
    void setStart(pixels* start);
    int getIndex(int x, int y);
public:
    pixels* start;
    pixels* goal;
    int pixelsTotal;
    set<pair<double, pair<int, int>>> openSet;
    vector<bool> visited;
    int max_x, max_y;
    vector<pixels> image;
    int heuristicType;
};

pixels::pixels()
{
    x = y = 0;
    f = g = 0.0;
    value = 0;
    parent = nullptr;
}

pixels::~pixels()
{

}

double pixels::getHeuristic(pixels* goal, int heuristicType)
{
    double heuristic;
    if (heuristicType == FOUR_DIRECTION)
    {
        heuristic = abs(x - goal->x) + abs(y - goal->y);
    }
    else if (heuristicType == EIGHT_DIRECTION)
    {
        heuristic = max(abs(x-goal->x), abs(y-goal->y));
    }
    else if (heuristicType == ANY_DIRECTION)
    {
        heuristic = sqrt( pow(x-goal->x, 2) + pow(y-goal->y, 2) );
    }
    else
    {
        cout << "ERROR: invalid heuristic type" << endl;
        return -1;
    }
    return heuristic;
}

bool pixels::operator==(pixels* goal)
{
    if (this->x = goal->x && this->y == goal->y)
        return true;
    return false;
}

AAsterisk::AAsterisk(Mat img, int heuristicType)
{
    max_x = img.cols - 1;
    max_y = img.rows - 1;
    pixelsTotal = img.cols * img.rows;
    for (int i = 0; i < img.rows; ++i)
    for (int j = 0; j < img.cols; ++j)
    {
        pixels px;
        px.f = DBL_MAX;
        px.g = 0;
        px.parent = nullptr;
        px.x = j;
        px.y = i;
        px.value = img.at<uchar>(i, j);
        image.push_back(px);
    }
    start = goal = nullptr;
    visited.resize(pixelsTotal, false);
    this->heuristicType = heuristicType;
}

AAsterisk::~AAsterisk()
{
    cout << "~AAsterisk() success" << endl;
}

bool AAsterisk::isValid(int x, int y)
{
    if (x < 0 || y < 0 || x > max_x || y > max_y) {
        return false;
    }
    int idx = getIndex(x, y);
    if (image[idx].value == 0){
        return false;
    }
    return true;
}

void AAsterisk::setGoal(pixels* goal)
{
    this->goal = goal;
}

void AAsterisk::setStart(pixels* start)
{
    this->start = start;
}

int AAsterisk::getIndex(int x, int y)
{
    int index = y * (max_x + 1) + x;
    return index;
}

bool AAsterisk::search()
{
    // reset from last search
    openSet.clear();
    for (int i = 0; i < pixelsTotal; ++i)
    {
        visited[i] = false;
        image[i].f = DBL_MAX;
        image[i].g = 0;
        image[i].parent = nullptr;
    }

    cout << "start: {x,y,value}={" << start->x << "," << start->y << "," << start->value << "}" << endl;
    cout << "goal: {x,y,value}={" << goal->x << "," << goal->y << "," << goal->value << "}" << endl;
    start->g = 0;
    double h = start->getHeuristic(goal, heuristicType);
    start->f = start->g + h;
    openSet.insert({start->f, {start->x, start->y}});
    while(!openSet.empty())
    {
        pair<int, int> pos = openSet.begin()->second;
        int index = getIndex(pos.first, pos.second);
        pixels* curr = &image[index];
        visited[index] = true;
        openSet.erase(openSet.begin());

        if (curr == goal)
        {
            return true;
        }

        for (int i = 0; i < 8; ++i)
        {
            if ( (heuristicType == FOUR_DIRECTION) && (i % 2 == 0) )
                continue;
            int x = curr->x + offset[i][0];
            int y = curr->y + offset[i][1];
            if (!isValid(x, y))
                continue;
            int idx = getIndex(x, y);
            pixels* neighbour = &image[idx];
            double gNew = curr->g + 1;
            double hNew = neighbour->getHeuristic(goal, heuristicType);
            double fNew = gNew + hNew;
            if (fNew < neighbour->f && !visited[getIndex(neighbour->x, neighbour->y)])
            {
                neighbour->parent = curr;
                neighbour->g = gNew;
                neighbour->f = fNew;
                openSet.insert({neighbour->f, {neighbour->x, neighbour->y}});
            }
        }
    }
    return false;
}


bool startCase = true;
int cnt = 0;
bool isNewTest = false;
void onMouse(int event, int x, int y, int, void* finder)
{
    if (event != EVENT_LBUTTONDOWN) return;
    
    AAsterisk* fd = (AAsterisk*)finder;
    int idx = fd->getIndex(x, y);
    pixels* px = &fd->image[idx];
    if (startCase)
    {
        src.at<Vec3b>(px->y, px->x) = Vec3b(0, 255, 0);
        fd->setStart(px);
        startCase = false;
    }
    else
    {
        fd->setGoal(px);
        startCase = true;
    }
    cnt++;
    if (cnt % 2 == 0) isNewTest = true;
}

// test
int main(int argc, char** argv)
{
    if (argc != 3)
    {
        cout << "==================================================" << endl;
        cout << "Usage:" << endl;
        cout << "\t./AAsterisk path/to/image <heuristic type>\n" << endl;
        cout << "\t\t0: 4 direction type" << endl;
        cout << "\t\t1: 8 direction type" << endl;
        cout << "\t\t2: any direction type" << endl;
        cout << "==================================================" << endl;
        return -1;
    }
    string outputWnd = "output";
    string filename = argv[1];
    int heuristicType = stoi(argv[2]);
    Mat img = imread(filename);
    if (img.empty())
    {
        cout << "cannot read image" << endl;
        return 1;
    }
    src = img.clone();
    if (heuristicType < 0 || heuristicType > 2)
    {
        cout << "only 0, 1, 2 is used" << endl;
        return 2;
    }

    cvtColor(img, img, CV_BGR2GRAY);
    AAsterisk* finder = new AAsterisk(img, heuristicType);
    namedWindow(outputWnd, WINDOW_NORMAL);
    setMouseCallback(outputWnd, onMouse, finder);

    for (;;)
    {
        imshow(outputWnd, src);
        if (waitKey(1) == 27) break;
        if (!isNewTest) continue;
        isNewTest = false;

        if ( finder->search() )
        {
            cout << "found path" << endl;
            pixels* curr = finder->goal;
            while (true)
            {
                src.at<Vec3b>(curr->y, curr->x) = Vec3b(0, 255, 0);
                if (curr == finder->start) break;
                curr = curr->parent;
            }
        }
        else
        {
            cout << "path not found!" << endl;
        }
    }
    delete finder;

    return 0;
}