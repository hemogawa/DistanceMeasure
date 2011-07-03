#include <iostream>
#include "cv.h"
#include "highgui.h"
#include <cstdio>
#include <cstdlib>

using namespace std;
using namespace cv;
bool first = true;

int detectAndDraw(Mat& img, CascadeClassifier& cascade, CascadeClassifier& nestCascade, double scale);
String cascadeName = "../../haarcascade_frontalface_alt.xml";
String nestedCascadeName = "../../haarcascade_eye_tree_eyeglasses.xml";

int main (int argc, char * const argv[]) {
	CvCapture* capture = 0;
	Mat frame, frameCopy, image;
	const String scaleOpt = "--scale=";
	size_t scaleOptLen = scaleOpt.length();
	const String cascadeOpt = "--cascade=";
	size_t cascadeOptLen = cascadeOpt.length();
	const String nestedCascadeOpt = "--nested-cascade";	//うしろに=つけないの?
	size_t nestedCascadeOptLen = nestedCascadeOpt.length();
	String inputName;
	FILE *fp;
	CascadeClassifier cascade, nestedCascade;
	double scale = 1;
	int width, baseWidth = 190, zoomeStep = 0;
	
	if(!cascade.load(cascadeName)){
		cerr << "ERROR: Could not load classifier cascade" << endl;
        cerr << "Usage: facedetect [--cascade=\"<cascade_path>\"]\n"
		" [--nested-cascade[=\"nested_cascade_path\"]]\n"
		" [--scale[=<image scale>\n"
		" [filename|camera_index]\n" ;
        return -1;
    }
    if( inputName.empty() || (isdigit(inputName.c_str()[0]) && inputName.c_str()[1] == '\0') )
        capture = cvCaptureFromCAM( inputName.empty() ? 0 : inputName.c_str()[0] - '0' );
    else if( inputName.size() )
    {
        image = imread( inputName, 1 );
        if( image.empty() )
            capture = cvCaptureFromAVI( inputName.c_str() );
    }
    else
        image = imread( "lena.jpg", 1 );
	
	fp = fopen("/Users/kikugawamariko/logs/distanceLog.txt","a");
	if( capture )
    {
        for(;;)
        {
            IplImage* iplImg = cvQueryFrame( capture );
            frame = iplImg;
            if( frame.empty() )
                break;
            if( iplImg->origin == IPL_ORIGIN_TL )
                frame.copyTo( frameCopy );
            else
                flip( frame, frameCopy, 0 );
			
            width = detectAndDraw( frameCopy, cascade, nestedCascade, scale );
			if (first == true) {
				baseWidth = width;
			} else {
				if((width-baseWidth)/20 >= zoomeStep+1){
					zoomeStep++;
					printf("ZoomeIn!%d:",zoomeStep);
					fprintf(fp,"%d\n",zoomeStep);
				}else if ((width-baseWidth)/20 < zoomeStep) {
					printf("ZoomeOut!%d:",zoomeStep);
					zoomeStep--;
					fprintf(fp,"%d\n",zoomeStep);
				}
			}
			first = false;
            if( waitKey( 10 ) >= 0 )
                goto _cleanup_;
			fflush(fp);
        }
        waitKey(0);
	_cleanup_:
        cvReleaseCapture( &capture );
    }
    else
    {
        if( !image.empty() )
        {
            width = detectAndDraw( image, cascade, nestedCascade, scale );
            waitKey(0);
        }
        else if( !inputName.empty() )
        {
            /* assume it is a text file containing the
			 list of the image filenames to be processed - one per line */
            FILE* f = fopen( inputName.c_str(), "rt" );
            if( f )
            {
                char buf[1000+1];
                while( fgets( buf, 1000, f ) )
                {
                    int len = (int)strlen(buf), c;
                    while( len > 0 && isspace(buf[len-1]) )
                        len--;
                    buf[len] = '\0';
                    cout << "file " << buf << endl;
                    image = imread( buf, 1 );
                    if( !image.empty() )
                    {
						width = detectAndDraw( image, cascade, nestedCascade, scale );
                        c = waitKey(0);
                        if( c == 27 || c == 'q' || c == 'Q' )
                            break;
                    }
                }
                fclose(f);
            }
        }
    }
	fclose(fp);
    return 0;
}

int detectAndDraw( Mat& img,
				  CascadeClassifier& cascade, CascadeClassifier& nestedCascade,
				  double scale)
{
    int i = 0,faceSize = 190;
    double t = 0;
    vector<Rect> faces;
    const static Scalar colors[] = { CV_RGB(0,0,255),
        CV_RGB(0,128,255),
        CV_RGB(0,255,255),
        CV_RGB(0,255,0),
        CV_RGB(255,128,0),
        CV_RGB(255,255,0),
        CV_RGB(255,0,0),
        CV_RGB(255,0,255)} ;
    Mat gray, smallImg( cvRound (img.rows/scale), cvRound(img.cols/scale), CV_8UC1 );
	
    cvtColor( img, gray, CV_BGR2GRAY );
    resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );
    equalizeHist( smallImg, smallImg );
	
    t = (double)cvGetTickCount();
    cascade.detectMultiScale( smallImg, faces,
							 1.1, 2, 0
							 //|CV_HAAR_FIND_BIGGEST_OBJECT
							 //|CV_HAAR_DO_ROUGH_SEARCH
							 |CV_HAAR_SCALE_IMAGE
							 ,
							 Size(190, 190) );
    t = (double)cvGetTickCount() - t;
    printf( "new detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
    for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ )
    {
		if (r->width > 190) {
			faceSize = r->width;
		}
    }
	return faceSize;
}