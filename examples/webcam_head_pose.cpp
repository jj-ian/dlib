// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt
/*

    This example program shows how to find frontal human faces in an image and
    estimate their pose.  The pose takes the form of 68 landmarks.  These are
    points on the face such as the corners of the mouth, along the eyebrows, on
    the eyes, and so forth.  
    

    This example is essentially just a version of the face_landmark_detection_ex.cpp
    example modified to use OpenCV's VideoCapture object to read from a camera instead 
    of files.


    Finally, note that the face detector is fastest when compiled with at least
    SSE2 instructions enabled.  So if you are using a PC with an Intel or AMD
    chip then you should enable at least SSE2 instructions.  If you are using
    cmake to compile this program you can enable them by using one of the
    following commands when you create the build project:
        cmake path_to_dlib_root/examples -DUSE_SSE2_INSTRUCTIONS=ON
        cmake path_to_dlib_root/examples -DUSE_SSE4_INSTRUCTIONS=ON
        cmake path_to_dlib_root/examples -DUSE_AVX_INSTRUCTIONS=ON
    This will set the appropriate compiler options for GCC, clang, Visual
    Studio, or the Intel compiler.  If you are using another compiler then you
    need to consult your compiler's manual to determine how to enable these
    instructions.  Note that AVX is the fastest but requires a CPU from at least
    2011.  SSE4 is the next fastest and is supported by most current machines.  
*/

#include <dlib/opencv.h>
#include <opencv2/opencv.hpp>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include "render_face.hpp"

#include <iostream>

//cpr stuff

#include <cpr/cpr.h>
//#include <json.hpp>
#include <string>

//end cpr stuff
using namespace dlib;
using namespace std;

#define FACE_DOWNSAMPLE_RATIO 2
#define SKIP_FRAMES 2
#define OPENCV_FACE_RENDER



std::vector<cv::Point3d> get_3d_model_points()
{
    std::vector<cv::Point3d> modelPoints;

    modelPoints.push_back(cv::Point3d(0.0f, 0.0f, 0.0f)); //The first must be (0,0,0) while using POSIT
    modelPoints.push_back(cv::Point3d(0.0f, -330.0f, -65.0f));
    modelPoints.push_back(cv::Point3d(-225.0f, 170.0f, -135.0f));
    modelPoints.push_back(cv::Point3d(225.0f, 170.0f, -135.0f));
    modelPoints.push_back(cv::Point3d(-150.0f, -150.0f, -125.0f));
    modelPoints.push_back(cv::Point3d(150.0f, -150.0f, -125.0f));
    
    return modelPoints;
    
}

//full_object_detection object - This object represents the location of an object in an image (as a rectangle) along with the positions of each of its constituent parts. Each part is a vector of points
std::vector<cv::Point2d> get_2d_image_points(full_object_detection &d)
{
    std::vector<cv::Point2d> image_points;
    image_points.push_back( cv::Point2d( d.part(30).x(), d.part(30).y() ) );    // Nose tip
    image_points.push_back( cv::Point2d( d.part(8).x(), d.part(8).y() ) );      // Chin
    image_points.push_back( cv::Point2d( d.part(36).x(), d.part(36).y() ) );    // Left eye left corner
    image_points.push_back( cv::Point2d( d.part(45).x(), d.part(45).y() ) );    // Right eye right corner
    image_points.push_back( cv::Point2d( d.part(48).x(), d.part(48).y() ) );    // Left Mouth corner
    image_points.push_back( cv::Point2d( d.part(54).x(), d.part(54).y() ) );    // Right mouth corner
    return image_points;

}

cv::Mat get_camera_matrix(float focal_length, cv::Point2d center)
{
    cv::Mat camera_matrix = (cv::Mat_<double>(3,3) << focal_length, 0, center.x, 0 , focal_length, center.y, 0, 0, 1);
    return camera_matrix;
}

int main()
{
    try
    {
        cv::VideoCapture cap(0);
        if (!cap.isOpened())
        {
            cerr << "Unable to connect to camera" << endl;
            return 1;
        }

        double fps = 30.0; // Just a place holder. Actual value calculated after 100 frames.
        cv::Mat im;
        
        // Get first frame and allocate memory.
        cap >> im;
        cv::Mat im_small, im_display;
        cv::resize(im, im_small, cv::Size(), 1.0/FACE_DOWNSAMPLE_RATIO, 1.0/FACE_DOWNSAMPLE_RATIO);

        cv::resize(im, im_display, cv::Size(), 0.5, 0.5);
        
        cv::Size size = im.size();

        std::cout << "bigim height " << im.size().height << std::endl;
        std::cout << "bigim width " << im.size().width << std::endl;

        std::cout << "im_display height " << im_display.size().height << std::endl;
        std::cout << "im_display width " << im_display.size().width << std::endl;

        std::cout << "smallim height " << im_small.size().height << std::endl;
        std::cout << "smallim width " << im_small.size().width << std::endl;
        



        
        
#ifndef OPENCV_FACE_RENDER 
        image_window win;
#endif

        // Load face detection and pose estimation models.
        frontal_face_detector detector = get_frontal_face_detector();
        shape_predictor pose_model;
        deserialize("shape_predictor_68_face_landmarks.dat") >> pose_model;

        int count = 0;
        std::vector<rectangle> faces;
        // Grab and process frames until the main window is closed by the user.
        double t = (double)cv::getTickCount();
#ifdef OPENCV_FACE_RENDER
        while(1)
#else
            while(!win.is_closed())
#endif
            {

                if ( count == 0 )
                    t = cv::getTickCount();
            // Grab a frame
                cap >> im;

            // Resize image for face detection
                cv::resize(im, im_small, cv::Size(), 1.0/FACE_DOWNSAMPLE_RATIO, 1.0/FACE_DOWNSAMPLE_RATIO);

            // Change to dlib's image format. No memory is copied.
                cv_image<bgr_pixel> cimg_small(im_small);
                cv_image<bgr_pixel> cimg(im);


            // Detect faces 
                if ( count % SKIP_FRAMES == 0 )
                {
                    //returns list of bounding boxes around all faces it can find in image
                    faces = detector(cimg_small);
                    std::cout << "num faces: " << faces.size() << std::endl;

                    auto payload = cpr::Payload{
                        {"faceCount", std::to_string(faces.size())}
                    };
                    
                    
                    // get the midpoint of each detected face, normalize it by the width of the frame, flip so MotionPortrait looks at you instead of with you, adn send it to server
                    for (int i = 0; i < faces.size(); i++) {
                        payload.AddPair({"facesX", std::to_string(
                            1 - (double(faces[i].left() + faces[i].right()) / 2 / im_small.size().width) 
                            )});
                        payload.AddPair({"facesY", std::to_string(
                            1 - (double(faces[i].top() + faces[i].bottom()) / 2 / im_small.size().height) 
                            )});

                    }
                    
                    payload.AddPair({"facesX", "-1"});
                    payload.AddPair({"facesY", "-1"});


                    //pickup, send faces positions as payload


                    /*
                    auto response = cpr::Post(cpr::Url{"localhost:8080/api"}, cpr::Payload{
                        {"faceCount", std::to_string(faces.size())}

                    });*/
                    auto response = cpr::Post(cpr::Url{"localhost:8080/api"}, payload);
                    std::cout << response.text << std::endl;
                }

            // Pose estimation
                // 3d locations of model landmarks
                std::vector<cv::Point3d> model_points = get_3d_model_points();


            // Find the pose of each face.
                std::vector<full_object_detection> shapes;
                //faces is list of bounding boxes
                for (unsigned long i = 0; i < faces.size(); ++i)
                {
                    rectangle r(
                        (long)(faces[i].left() * FACE_DOWNSAMPLE_RATIO),
                        (long)(faces[i].top() * FACE_DOWNSAMPLE_RATIO),
                        (long)(faces[i].right() * FACE_DOWNSAMPLE_RATIO),
                        (long)(faces[i].bottom() * FACE_DOWNSAMPLE_RATIO)
                        );
                    full_object_detection shape = pose_model(cimg, r);
                    shapes.push_back(shape);
#ifdef OPENCV_FACE_RENDER
                    render_face(im, shape);
                    std::vector<cv::Point2d> image_points = get_2d_image_points(shape);
                    double focal_length = im.cols;
                    cv::Mat camera_matrix = get_camera_matrix(focal_length, cv::Point2d(im.cols/2,im.rows/2));
                    cv::Mat rotation_vector;
                    cv::Mat rotation_matrix;
                    cv::Mat translation_vector;


                    cv::Mat dist_coeffs = cv::Mat::zeros(4,1,cv::DataType<double>::type);

                    cv::solvePnP(model_points, image_points, camera_matrix, dist_coeffs, rotation_vector, translation_vector);

                //cv::Rodrigues(rotation_vector, rotation_matrix);

                    std::vector<cv::Point3d> nose_end_point3D;
                    std::vector<cv::Point2d> nose_end_point2D;
                    nose_end_point3D.push_back(cv::Point3d(0,0,1000.0));

                    cv::projectPoints(nose_end_point3D, rotation_vector, translation_vector, camera_matrix, dist_coeffs, nose_end_point2D);		
//                cv::Point2d projected_point = find_projected_point(rotation_matrix, translation_vector, camera_matrix, cv::Point3d(0,0,1000.0));
                    //image_points[0] is the x,y coordinate at the tip of user's nose
                    //nose_end_point2d[0] is the x,y coordinate at the end of the vector that extends from user's nose 
                    
                    std::cout << "imgpt: " << image_points[0].x << std::endl;
                    std::cout << "nosept: " << nose_end_point2D[0].x << std::endl;

                    cv::line(im,image_points[0], nose_end_point2D[0], cv::Scalar(255,0,0), 2);
//                cv::line(im,image_points[0], projected_point, cv::Scalar(0,0,255), 2);

                    //cpr test

                   // im.size().height << std::endl;
                    //normalize endpoint for motionportrait
                    auto normalizedNoseEndPointX = nose_end_point2D[0].x / im.size().width;
                    std::cout << "norm nose end point x " << normalizedNoseEndPointX << std::endl;
                    auto normalizedNoseEndPointY = 1-(nose_end_point2D[0].y / im.size().height);
                    std::cout << "norm nose end point y " << normalizedNoseEndPointY << std::endl;

                    
                    auto response = cpr::Post(cpr::Url{"localhost:8080/api"},
                        cpr::Payload{{"x", std::to_string(normalizedNoseEndPointX) },{"y", std::to_string(normalizedNoseEndPointY) },{"action","lookAt"}});
                    std::cout << response.text << std::endl;




#endif
                }
        		// Uncomment the line below to see FPS    
                cv::putText(im, cv::format("fps %.2f",fps), cv::Point(50, size.height - 50), cv::FONT_HERSHEY_COMPLEX, 1.5, cv::Scalar(0, 0, 255), 3);


            // Display it all on the screen
#ifdef OPENCV_FACE_RENDER

                // Resize image for display
                im_display = im;
                cv::resize(im, im_display, cv::Size(), 0.5, 0.5);
                cv::imshow("Fast Facial Landmark Detector", im_display);

                // WaitKey slows down the runtime quite a lot
                // So check every 15 frames


                if ( count % 15 == 0)
                {
                    int k = cv::waitKey(1);
                    // Quit if 'q' or ESC is pressed
                    if ( k == 'q' || k == 27)
                    {
                        return 0;
                    }
                }



#else

                win.clear_overlay();
                win.set_image(cimg);
                win.add_overlay(render_face_detections(shapes));
#endif

                count++;

                if ( count == 100)
                {
                    t = ((double)cv::getTickCount() - t)/cv::getTickFrequency();
                    fps = 100.0/t;
                    count = 0;
                }



            }
        }
        catch(serialization_error& e)
        {
            cout << "You need dlib's default face landmarking model file to run this example." << endl;
            cout << "You can get it from the following URL: " << endl;
            cout << "   http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2" << endl;
            cout << endl << e.what() << endl;
        }
        catch(exception& e)
        {
            cout << e.what() << endl;
        }
    }

